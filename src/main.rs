use clap::Parser;
use crossbeam_channel::bounded;
use tracing_subscriber::{EnvFilter, fmt, prelude::*};

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

    #[arg(long, value_enum, default_value_t = eyesthatblink::blink_detector::BlinkMethod::AdaptiveEar)]
    blink_method: eyesthatblink::blink_detector::BlinkMethod,
}

fn main() {
    tracing_subscriber::registry()
        .with(fmt::layer())
        .with(EnvFilter::from_default_env())
        .init();

    let cli = Cli::parse();
    tracing::debug!("CLI arguments: {:?}", cli);

    // create channels for cross-thread communication
    let (frame_tx, frame_rx) = bounded(10);
    let (blink_tx, blink_rx) = bounded(10);

    // launch the camera, read frame and send them to the blink detector.
    let mut camera = eyesthatblink::Camera::new(0);
    let _thread_frame = std::thread::spawn(move || {
        camera.start(frame_tx);
    });

    // Launhch the blink detector, receive frame from camera and send blink event to manager.
    let _thread_blink = std::thread::spawn(move || {
        let mut blink_detector =
            eyesthatblink::BlinkDetector::new(frame_rx, blink_tx, cli.blink_method);
        blink_detector.start(cli.draw);
    });

    // Manage what to do when we receive a blink event e.g. notify, log, etc.
    let mut manager = eyesthatblink::Manager::new(blink_rx);
    manager.start();
}
