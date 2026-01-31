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
}

fn main() {
    tracing_subscriber::registry()
        .with(fmt::layer())
        .with(EnvFilter::from_default_env())
        .init();

    let cli = Cli::parse();
    tracing::debug!("CLI arguments: {:?}", cli);

    let mut camera = eyesthatblink::Camera::new(0);

    let (frame_tx, frame_rx) = bounded(10);
    let (blink_tx, blink_rx) = bounded(10);

    let _thread_frame = std::thread::spawn(move || {
        camera.start(frame_tx);
    });

    let _thread_blink = std::thread::spawn(move || {
        let mut blink_detector = eyesthatblink::BlinkDetector::new(frame_rx, blink_tx);
        blink_detector.start(cli.draw);
    });

    // main thread the process the blink.
    loop {
        for blink in blink_rx.iter() {
            println!("Blink detected: {:?}", blink);
        }
        std::thread::sleep(std::time::Duration::from_millis(100));
    }
}
