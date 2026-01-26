use opencv::prelude::*;
use opencv::videoio;

pub(crate) struct Camera {
    camera: videoio::VideoCapture,
    window: Option<String>,
}

impl Camera {
    pub fn new(camera_id: i32, window: Option<String>) -> Self {
        let camera =
            videoio::VideoCapture::new(camera_id, videoio::CAP_ANY).expect("failed to open camera");
        Self { camera, window }
    }

    pub fn start(&mut self) {
        if let Err(e) = self.start_inner() {
            tracing::error!("Camera error: {e}");
            return self.start();
        }
    }

    fn start_inner(&mut self) -> anyhow::Result<()> {
        tracing::info!("Starting camera");
        loop {
            let mut frame = Mat::default();
            let result = self.camera.read(&mut frame)?;
            if !result {
                tracing::warn!("No frame captured from camera");
                std::thread::sleep(std::time::Duration::from_millis(100));
                continue;
            }

            if let Err(e) = self.show_frame(&frame) {
                tracing::warn!("failed to show frame: {e}");
            }
            std::thread::sleep(std::time::Duration::from_millis(10));
        }
    }

    fn show_frame(&self, frame: &Mat) -> anyhow::Result<()> {
        if self.window.is_none() {
            return Ok(());
        }

        opencv::highgui::imshow(self.window.as_ref().expect("window already exists"), frame)?;
        let key = opencv::highgui::wait_key(1)?;
        if key > 0 && key != 255 {
            std::process::exit(0);
        }

        Ok(())
    }
}
