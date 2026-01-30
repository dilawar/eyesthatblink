use crossbeam_channel::Sender;
use opencv::core::{Mat, Size};
use opencv::imgproc;
use opencv::prelude::*;
use opencv::videoio;

pub(crate) struct Camera {
    camera: videoio::VideoCapture,
}

impl Camera {
    pub fn new(camera_id: i32) -> Self {
        let camera =
            videoio::VideoCapture::new(camera_id, videoio::CAP_ANY).expect("failed to open camera");
        Self { camera }
    }

    pub fn start(&mut self, tx: Sender<Mat>) {
        loop {
            if let Err(e) = self.start_inner(&tx) {
                tracing::error!("Camera error: {e}. Sleeping for 1 second before retrying.");
                std::thread::sleep(std::time::Duration::from_millis(1000));
            }
        }
    }

    // 5 to 10 frames per second is more than enough for blink detection.
    fn start_inner(&mut self, tx: &Sender<Mat>) -> anyhow::Result<()> {
        const NUM_COLS: f64 = 480.0;
        loop {
            let mut frame = Mat::default();
            let result = self.camera.read(&mut frame)?;
            if !result {
                tracing::warn!("No frame captured from camera. Sleeping for 5s.");
                std::thread::sleep(std::time::Duration::from_millis(5000));
                continue;
            }

            // resize the frame to 256 px width
            let mut resized = Mat::default();
            let frac = NUM_COLS / (frame.cols() as f64);

            imgproc::resize(
                &frame,
                &mut resized,
                Size::new(0, 0),
                frac,
                frac,
                imgproc::INTER_LINEAR,
            )?;

            tx.send(resized)
                .map_err(|e| anyhow::anyhow!("failed to send frame: {}", e))?;

            std::thread::sleep(std::time::Duration::from_millis(150));
        }
    }
}
