use crate::config::Config;
use crossbeam_channel::Sender;
use opencv::core::Mat;
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

    pub fn start(&mut self, config: &Config, tx: Sender<Mat>) {
        loop {
            if let Err(e) = self.start_inner(config, &tx) {
                tracing::error!("Camera error: {e}. Sleeping for 1 second before retrying.");
                std::thread::sleep(std::time::Duration::from_millis(1000));
            }
        }
    }

    fn start_inner(&mut self, config: &Config, tx: &Sender<Mat>) -> anyhow::Result<()> {
        tracing::info!("Starting camera, config={:?}", config);
        loop {
            let mut frame = Mat::default();
            let result = self.camera.read(&mut frame)?;
            if !result {
                tracing::warn!("No frame captured from camera");
                std::thread::sleep(std::time::Duration::from_millis(100));
                continue;
            }

            tx.send(frame)
                .map_err(|e| anyhow::anyhow!("failed to send frame: {}", e))?;

            std::thread::sleep(std::time::Duration::from_millis(10));
        }
    }
}
