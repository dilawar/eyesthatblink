use crossbeam_channel::Receiver;
use opencv::core::Mat;

pub(crate) struct BlinkDetector {
    rx: Receiver<Mat>,
}

impl BlinkDetector {
    pub fn new(rx: Receiver<Mat>) -> Self {
        Self { rx }
    }

    pub fn start(&mut self) {
        // analyse frames.
        loop {
            if let Ok(frame) = self.rx.recv() {
                crate::util::show_frame(&frame).expect("Failed to show frame");
            }
        }
    }
}
