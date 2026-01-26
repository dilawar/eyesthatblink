use clap::Parser;
use std::path::PathBuf;
use tracing_subscriber::{EnvFilter, fmt, prelude::*};

mod camera;
use camera::Camera;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Cli {
    /// Turn debugging information on
    #[arg(short, long, action = clap::ArgAction::Count)]
    debug: u8,

    #[arg(short = 'm', long, default_value = "face_detection_yunet_2022mar.onnx")]
    fd_model_path: PathBuf,

    #[arg(
        short = 'r',
        long,
        default_value = "face_recognition_sface_2022mar.onnx"
    )]
    fr_model_path: PathBuf,

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

    let window = String::from("Eyes That Blink");
    opencv::highgui::named_window(&window, opencv::highgui::WINDOW_AUTOSIZE)
        .expect("Failed to create window");

    let mut camera = Camera::new(0, Some(window));
    camera.start();
}
