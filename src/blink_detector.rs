use crossbeam_channel::{Receiver, Sender};
use opencv::core::*;
use opencv::imgproc;
use opencv::objdetect;
use opencv::prelude::*;
use std::time::{Duration, Instant};

#[derive(Debug, Clone, Copy, PartialEq, Eq, clap::ValueEnum)]
pub enum BlinkMethod {
    Legacy,
    AdaptiveEar,
}

#[derive(Debug, Default)]
struct AdaptiveState {
    baseline_open_score: Option<f32>,
    in_closed_state: bool,
    closed_started_at: Option<Instant>,
    last_blink_at: Option<Instant>,
}

pub struct BlinkDetector {
    rx: Receiver<Mat>,
    blink_tx: Sender<crate::BlinkEvent>,
    face_detector: objdetect::CascadeClassifier,
    eye_detector: objdetect::CascadeClassifier,
    method: BlinkMethod,
    adaptive_state: AdaptiveState,
}

impl BlinkDetector {
    pub fn new(
        rx: Receiver<Mat>,
        blink_tx: Sender<crate::BlinkEvent>,
        method: BlinkMethod,
    ) -> Self {
        let cascade_ff = include_str!("../cascades/haarcascade_frontalface_default.xml");
        let cascade_eye = include_str!("../cascades/haarcascade_eye.xml");

        // write cascades to temp files
        std::fs::write(".face.xml", cascade_ff).expect("failed to write face cascade");
        std::fs::write(".eye.xml", cascade_eye).expect("failed to write eye cascade");

        let face_detector =
            objdetect::CascadeClassifier::new(".face.xml").expect("failed to load face detector");
        let eye_detector =
            objdetect::CascadeClassifier::new(".eye.xml").expect("failed to load eye detector");

        Self {
            rx,
            blink_tx,
            face_detector,
            eye_detector,
            method,
            adaptive_state: AdaptiveState::default(),
        }
    }

    pub fn start(&mut self, draw_frames: bool) {
        // analyse frames.
        // The consumer is not sending more than 5-10 frames a second.
        loop {
            if let Ok(frame) = self.rx.recv() {
                self.step(frame, draw_frames)
                    .expect("Failed to process frame");

                std::thread::sleep(std::time::Duration::from_millis(10));
            }
        }
    }

    fn detect_faces(&mut self, gray: &Mat) -> anyhow::Result<Vector<Rect>> {
        let mut faces = Vector::<Rect>::new();
        self.face_detector.detect_multi_scale(
            &gray,
            &mut faces,
            1.2,
            4,
            0,
            Size::new(30, 30),
            Size::new(0, 0),
        )?;

        Ok(faces)
    }

    fn detect_eyes(
        &mut self,
        gray: &mut Mat,
        face: Rect,
        draw_frames: bool,
    ) -> anyhow::Result<Vector<Rect>> {
        let mut eyes = Vector::<Rect>::new();
        let roi_gray = Mat::roi(gray, face)?;
        self.eye_detector.detect_multi_scale(
            &roi_gray,
            &mut eyes,
            1.1,
            2,
            0,
            Size::new(20, 20),
            Size::new(0, 0),
        )?;

        if draw_frames {
            let eyes_with_offset = eyes
                .iter()
                .map(|eye| Rect::new(eye.x + face.x, eye.y + face.y, eye.width, eye.height))
                .collect::<Vector<Rect>>();

            draw_rects(gray, &eyes_with_offset, 1.into(), 1);
        }

        Ok(eyes)
    }

    fn measure_eye_openness(eye_roi_gray: &impl ToInputArray) -> anyhow::Result<f32> {
        let mut normalized = Mat::default();
        imgproc::equalize_hist(eye_roi_gray, &mut normalized)?;

        let mut blurred = Mat::default();
        imgproc::gaussian_blur(
            &normalized,
            &mut blurred,
            Size::new(5, 5),
            0.0,
            0.0,
            BORDER_DEFAULT,
        )?;

        let mut binary = Mat::default();
        imgproc::threshold(
            &blurred,
            &mut binary,
            0.0,
            255.0,
            imgproc::THRESH_BINARY_INV | imgproc::THRESH_OTSU,
        )?;

        let mut non_zero = Vector::<Point>::new();
        find_non_zero(&binary, &mut non_zero)?;
        if non_zero.is_empty() {
            return Ok(0.0);
        }

        let eyelid_band = imgproc::bounding_rect(&non_zero)?;
        Ok(eyelid_band.height as f32 / eyelid_band.width.max(1) as f32)
    }

    fn detect_blinks_adaptive(&mut self, eyes: &Vector<Rect>, gray: &Mat) -> anyhow::Result<()> {
        const CLOSED_FOR_TOO_LONG: Duration = Duration::from_millis(900);
        const MIN_BLINK_DURATION: Duration = Duration::from_millis(60);
        const MAX_BLINK_DURATION: Duration = Duration::from_millis(550);
        const MIN_INTER_BLINK_INTERVAL: Duration = Duration::from_millis(120);

        let now = Instant::now();
        let mut openness_scores = Vec::with_capacity(eyes.len());
        for eye in eyes {
            let eye_rect = Rect::new(eye.x, eye.y, eye.width, eye.height);
            let eye_roi = Mat::roi(gray, eye_rect)?;
            openness_scores.push(Self::measure_eye_openness(&eye_roi)?);
        }

        let mean_open_score = if openness_scores.is_empty() {
            None
        } else {
            Some(openness_scores.iter().sum::<f32>() / openness_scores.len() as f32)
        };

        if let Some(score) = mean_open_score {
            let baseline = self.adaptive_state.baseline_open_score.unwrap_or(score);
            let alpha = if score > baseline * 0.65 { 0.08 } else { 0.02 };
            let new_baseline = baseline * (1.0 - alpha) + score * alpha;
            self.adaptive_state.baseline_open_score = Some(new_baseline.max(0.01));

            let closed_threshold = new_baseline * 0.55;
            let open_threshold = new_baseline * 0.75;

            if !self.adaptive_state.in_closed_state && score < closed_threshold {
                self.adaptive_state.in_closed_state = true;
                self.adaptive_state.closed_started_at = Some(now);
            } else if self.adaptive_state.in_closed_state && score > open_threshold {
                if let Some(started) = self.adaptive_state.closed_started_at {
                    let closed_duration = now.saturating_duration_since(started);
                    let enough_gap = self
                        .adaptive_state
                        .last_blink_at
                        .map(|t| now.saturating_duration_since(t) > MIN_INTER_BLINK_INTERVAL)
                        .unwrap_or(true);
                    if closed_duration >= MIN_BLINK_DURATION
                        && closed_duration <= MAX_BLINK_DURATION
                        && enough_gap
                    {
                        self.blink_tx
                            .send(crate::BlinkEvent { when: now })
                            .expect("failed to send blink event");
                        self.adaptive_state.last_blink_at = Some(now);
                    }
                }

                self.adaptive_state.in_closed_state = false;
                self.adaptive_state.closed_started_at = None;
            }
        } else if !self.adaptive_state.in_closed_state
            && self.adaptive_state.baseline_open_score.is_some()
        {
            self.adaptive_state.in_closed_state = true;
            self.adaptive_state.closed_started_at = Some(now);
        }

        if self.adaptive_state.in_closed_state
            && self
                .adaptive_state
                .closed_started_at
                .map(|t| now.saturating_duration_since(t) > CLOSED_FOR_TOO_LONG)
                .unwrap_or(false)
        {
            self.adaptive_state.in_closed_state = false;
            self.adaptive_state.closed_started_at = None;
        }

        Ok(())
    }

    /// Implements most of blink detection logic here.
    fn step(&mut self, frame: Mat, draw_frames: bool) -> anyhow::Result<()> {
        let mut gray = Mat::default();
        imgproc::cvt_color(&frame, &mut gray, imgproc::COLOR_BGR2GRAY, 0)?;

        let faces = self.detect_faces(&gray)?;

        if draw_frames {
            draw_rects(&mut gray, &faces, 1.into(), 1);
        }

        for face in faces {
            // Define ROI for eyes (upper half of face)
            let eyes = self.detect_eyes(&mut gray, face, draw_frames)?;
            self.detect_blinks(&eyes, &gray)?;
        }

        if draw_frames {
            crate::util::show_frame(&gray).expect("Failed to show frame");
        }

        Ok(())
    }

    fn detect_blinks(&mut self, eyes: &Vector<Rect>, _gray: &Mat) -> anyhow::Result<()> {
        if self.method == BlinkMethod::Legacy {
            if eyes.len() < 2 {
                // consider it a blink.
                self.blink_tx
                    .send(crate::BlinkEvent {
                        when: std::time::Instant::now(),
                    })
                    .expect("failed to send blink event");
            }
        } else {
            self.detect_blinks_adaptive(eyes, _gray)?;
        }

        Ok(())
    }
}

// Draw rectangles around detected eyes
fn draw_rects(frame: &mut Mat, eyes: &Vector<Rect>, color: Scalar, thickness: i32) {
    for eye in eyes {
        let eye_rect = Rect::new(eye.x, eye.y, eye.width, eye.height);

        imgproc::rectangle(frame, eye_rect, color, thickness, imgproc::LINE_8, 0)
            .expect("failed to draw_ eye");
    }
}
