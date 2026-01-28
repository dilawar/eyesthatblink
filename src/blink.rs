use crossbeam_channel::Receiver;
use opencv::core::Mat;
use opencv::imgproc;
use opencv::objdetect;

pub(crate) struct BlinkDetector {
    rx: Receiver<Mat>,
    face_detector: objdetect::CascadeClassifier,
    eye_detector: objdetect::CascadeClassifier,
}

impl BlinkDetector {
    pub fn new(rx: Receiver<Mat>) -> Self {
        let face_detector =
            objdetect::CascadeClassifier::new("haarcascade_frontalface_default.xml")
                .expect("failed to load face detector");
        let eye_detector = objdetect::CascadeClassifier::new("haarcascade_eye.xml")
            .expect("failed to load eye detector");

        Self {
            rx,
            face_detector,
            eye_detector,
        }
    }

    pub fn start(&mut self) {
        // analyse frames.
        loop {
            if let Ok(frame) = self.rx.recv() {
                self.step(frame).expect("Failed to process frame");
            }
        }
    }

    fn step(&mut self, frame: Mat) -> anyhow::Result<()> {
        let mut gray = Mat::default();
        imgproc::cvt_color(&frame, &mut gray, imgproc::COLOR_BGR2GRAY, 0)?;
        crate::util::show_frame(&gray).expect("Failed to show frame");
        Ok(())
    }
}
