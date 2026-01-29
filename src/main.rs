use clap::Parser;
use crossbeam_channel::bounded;
use tracing_subscriber::{EnvFilter, fmt, prelude::*};

mod blink;
mod camera;
mod util;

use camera::Camera;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Cli {
    /// Turn debugging information on
    #[arg(short, long, action = clap::ArgAction::Count)]
    debug: u8,

    #[arg(long, default_value_t = false)]
    draw: bool,

    #[arg(short, long, default_value = "0.9")]
    score_threshold: f32,

    #[arg(short, long, default_value = "0.3")]
    nms_threshold: f32,

    #[arg(short, long, default_value = "5000")]
    tok_k: u32,
}

fn main() {
    tracing_subscriber::registry()
        .with(fmt::layer())
        .with(EnvFilter::from_default_env())
        .init();

    let cli = Cli::parse();
    tracing::debug!("CLI arguments: {:?}", cli);

    let mut camera = Camera::new(0);
    let (tx, rx) = bounded(10);

    std::thread::spawn(move || {
        camera.start(tx);
    });

    let mut blink_detector = blink::BlinkDetector::new(rx);
    blink_detector.start(cli.draw);
}
