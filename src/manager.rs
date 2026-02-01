//! What to do when we recieve a blink.

use crate::BlinkEvent;
use circular_buffer::CircularBuffer;
use crossbeam_channel::Receiver;
use notify_rust::Notification;

pub struct Manager {
    blink_rx: Receiver<BlinkEvent>,
    blink_events: CircularBuffer<100, BlinkEvent>,
    last_notification: std::time::Instant,
}

impl Manager {
    const MAX_BLINK_INTERVAL_SECS: u64 = 30;
    const MIN_BLINKS_PER_MIN: f64 = 15.0;

    pub fn new(blink_rx: Receiver<BlinkEvent>) -> Self {
        Self {
            blink_rx,
            blink_events: CircularBuffer::new(),
            last_notification: std::time::Instant::now(),
        }
    }

    pub fn start(&mut self) {
        let mut last_tick = std::time::Instant::now();
        loop {
            // Do not wait for more than 5s for a blink event to arrive.
            if let Ok(blink) = self
                .blink_rx
                .recv_timeout(std::time::Duration::from_secs(1))
            {
                tracing::info!("Received blink event {:?}", blink);
                self.blink_events.push_back(blink);
            }

            let now = std::time::Instant::now();
            if (now - last_tick).as_secs() > 5 {
                if let Err(e) = self.take_action(now) {
                    tracing::error!("Error taking action: {:?}", e);
                }
                last_tick = std::time::Instant::now();
            }
        }
    }

    fn take_action(&mut self, now: std::time::Instant) -> anyhow::Result<()> {
        tracing::debug!("Checking if any action is needed...");
        if self.blink_events.is_empty() {
            tracing::debug!("No blink events recorded yet.");
            return Ok(());
        }

        // compute frequency of blinks
        let first_blink = self.blink_events.front().expect("must not be empty");
        let last_blink = self.blink_events.back().expect("must not be empty");

        // check when we blinked last.
        let dt = (last_blink.when - first_blink.when).as_secs_f64();
        let blinks_per_minute = if dt > 0.0 {
            (self.blink_events.len() as f64 * 60.0) / dt
        } else {
            0.0
        };

        tracing::info!("blinks_per_minute {}", blinks_per_minute);
        if blinks_per_minute < Self::MIN_BLINKS_PER_MIN
            && (now - self.last_notification) > std::time::Duration::from_secs(5 * 60)
        {
            // send notification if previous notification was more than 5 mins ago.
            self.send_notification(
                    "Blink Reminder",
                    &format!(
                        "You've not blinked in the last {} minutes! Please blink to keep your eyes healthy.",
                        Self::MIN_BLINKS_PER_MIN
                    ),
                )?;
            self.last_notification = now;
        }

        if (now - last_blink.when).as_secs() > Self::MAX_BLINK_INTERVAL_SECS {
            self.send_notification(
                "Blink Reminder",
                &format!(
                    "You've not blinked in the last {} seconds! Please blink to keep your eyes healthy.",
                    Self::MAX_BLINK_INTERVAL_SECS
                ),
            )?;
        }

        Ok(())
    }

    fn send_notification(&self, title: &str, message: &str) -> anyhow::Result<()> {
        Notification::new().summary(title).body(message).show()?;
        Ok(())
    }
}
