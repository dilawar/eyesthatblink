use crossbeam_channel::Receiver;
use opencv::core::*;
use opencv::imgproc;
use opencv::objdetect;
use opencv::prelude::*;
use rbl_circular_buffer::CircularBuffer;

pub struct BlinkDetector {
    rx: Receiver<Mat>,
    face_detector: objdetect::CascadeClassifier,
    eye_detector: objdetect::CascadeClassifier,
    blink_timestamp: CircularBuffer<std::time::Instant>,
}

impl BlinkDetector {
    pub fn new(rx: Receiver<Mat>) -> Self {
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
            face_detector,
            eye_detector,
            blink_timestamp: CircularBuffer::new(50),
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
            let blinks = self.detect_blinks(&eyes, &gray)?;
            println!("Detected {} eyes, {:?} blinks", eyes.len(), blinks);
        }

        if draw_frames {
            crate::util::show_frame(&gray).expect("Failed to show frame");
        }

        Ok(())
    }

    fn detect_blinks(&mut self, eyes: &Vector<Rect>, _gray: &Mat) -> anyhow::Result<()> {
        if eyes.len() < 2 {
            // consider it a blink.
            self.blink_timestamp.push(std::time::Instant::now());
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
