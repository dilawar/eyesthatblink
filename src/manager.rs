//! What to do when we recieve a blink.

use circular_buffer::CircularBuffer;
use crossbeam_channel::Receiver;
use crate::BlinkEvent;
        use notify_rust::Notification;

pub struct Manager {
    blink_rx: Receiver<BlinkEvent>,
    blink_events: CircularBuffer<100, BlinkEvent>,
}

impl Manager {

    const MAX_BLINK_INTERVAL_SECS: u64 = 20;
    const MIN_BLINKS_PER_MIN: u64 = 15;

    pub fn new(blink_rx: Receiver<BlinkEvent>) -> Self {
        Self { blink_rx, blink_events: CircularBuffer::new() }
    }

    pub fn start(&mut self) {
        let mut last_tick = std::time::Instant::now();
        loop {

            // Do not wait for more than 5s for a blink event to arrive.
            if let Ok(blink) = self.blink_rx.recv_timeout(std::time::Duration::from_secs(5)) {
                self.blink_events.push_back(blink);
            }

            let now = std::time::Instant::now();
            if (now - last_tick).as_secs() > 20 {
                tracing::info!("Checking if action is needed...");
                if let Err(e) = self.take_action(now) {
                    tracing::error!("Error taking action: {:?}", e);
                }
                last_tick = std::time::Instant::now();
            }

        }
    }

    fn take_action(&self, now: std::time::Instant) -> anyhow::Result<()> {
        // check when we blinked last.
        if let Some(last_blink) = self.blink_events.back() {
            tracing::info!("Last blink at {:?}", last_blink);
            if (now - last_blink.when).as_secs() > Self::MAX_BLINK_INTERVAL_SECS { 
                Notification::new()
                    .summary("Blink Reminder")
                    .body(&format!("You've not blinked in the last {} seconds! Please blink to keep your eyes healthy.", Self::MAX_BLINK_INTERVAL_SECS))
                    .show()?;
            }
        }

        Ok(())
    }
}
