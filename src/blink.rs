use crossbeam_channel::Receiver;
use opencv::core::*;
use opencv::imgproc;
use opencv::objdetect;
use opencv::prelude::*;

pub(crate) struct BlinkDetector {
    rx: Receiver<Mat>,
    face_detector: objdetect::CascadeClassifier,
    eye_detector: objdetect::CascadeClassifier,
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
        }
    }

    pub fn start(&mut self, draw_frames: bool) {
        // analyse frames.
        loop {
            if let Ok(frame) = self.rx.try_recv() {
                self.step(frame, draw_frames)
                    .expect("Failed to process frame");
                std::thread::sleep(std::time::Duration::from_millis(100));
            }
        }
    }

    fn step(&mut self, frame: Mat, draw_frames: bool) -> anyhow::Result<()> {
        let mut gray = Mat::default();
        imgproc::cvt_color(&frame, &mut gray, imgproc::COLOR_BGR2GRAY, 0)?;

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

        if draw_frames {
            draw_rects(&mut gray, &faces, 1.into(), 1);
        }

        for face in faces {
            // Define ROI for eyes (upper half of face)
            let roi_gray = Mat::roi(&gray, face)?;
            let mut eyes = Vector::<Rect>::new();
            self.eye_detector.detect_multi_scale(
                &roi_gray,
                &mut eyes,
                1.1,
                2,
                0,
                Size::new(20, 20),
                Size::new(0, 0),
            )?;

            let eyes_with_offset = eyes
                .iter()
                .map(|eye| Rect::new(eye.x + face.x, eye.y + face.y, eye.width, eye.height))
                .collect::<Vector<Rect>>();

            if draw_frames {
                draw_rects(&mut gray, &eyes_with_offset, 1.into(), 1);
            }
        }

        if draw_frames {
            crate::util::show_frame(&gray).expect("Failed to show frame");
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
