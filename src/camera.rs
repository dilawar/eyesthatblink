use opencv::prelude::*;
use opencv::videoio;

#[derive(Default)]
pub(crate) struct Camera {
    camera: Option<videoio::VideoCapture>,
}

impl Camera {
    pub fn new(camera_id: i32) -> Self {
        let camera = videoio::VideoCapture::new(camera_id, videoio::CAP_ANY)
            .inspect_err(|e| tracing::error!("Failed to create VideoCapture: {e}"))
            .ok();

        Self { camera }
    }

    pub fn start(&mut self) {
        if self.camera.is_none() {
            tracing::error!("No camera found");

            return;
        }

        let camera = self.camera.as_mut().expect("camera already exists");
        // if ! videoio::VideoCapture::is_opened(&mut camera).expect("failed to check if camera is opened") {
        //     tracing::error!("Camera could not be opened.");
        //     return;
        // }

        tracing::info!("Starting camera");
        loop {
            let mut frame = Mat::default();
            match camera.read(&mut frame) {
                Ok(true) => {
                    tracing::info!("Frame read {frame:?}");
                    std::thread::sleep(std::time::Duration::from_millis(10));
                }
                Ok(false) => {
                    tracing::info!("Frame could not be read.");
                    std::thread::sleep(std::time::Duration::from_millis(100));
                }
                Err(e) => {
                    tracing::error!("Failed to read frame: {e}");
                    std::thread::sleep(std::time::Duration::from_millis(1000));
                }
            }
        }
    }
}
