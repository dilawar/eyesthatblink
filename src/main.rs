use clap::Parser;
use crossbeam_channel::bounded;
use std::path::PathBuf;
use tracing_subscriber::{EnvFilter, fmt, prelude::*};

mod blink;
mod camera;
mod config;
mod util;

use camera::Camera;
use config::Config;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Cli {
    /// Turn debugging information on
    #[arg(short, long, action = clap::ArgAction::Count)]
    debug: u8,

    #[arg(short, long, default_value = "0.9")]
    score_threshold: f32,

    #[arg(short, long, default_value = "0.3")]
    nms_threshold: f32,

    #[arg(short, long, default_value = "5000")]
    tok_k: u32,
}

impl Into<Config> for Cli {
    fn into(self) -> Config {
        Config {
            score_threshold: self.score_threshold,
            nms_threshold: self.nms_threshold,
            tok_k: self.tok_k,
        }
    }
}

fn main() {
    tracing_subscriber::registry()
        .with(fmt::layer())
        .with(EnvFilter::from_default_env())
        .init();

    let cli = Cli::parse();
    tracing::debug!("CLI arguments: {:?}", cli);

    let config: Config = cli.into();
    tracing::debug!("config : {:?}", config);

    let mut camera = Camera::new(0);
    let (tx, rx) = bounded(10);

    std::thread::spawn(move || {
        camera.start(&config, tx);
    });

    let mut blink_detector = blink::BlinkDetector::new(rx);
    blink_detector.start();
}
