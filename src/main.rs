use clap::Parser;
use std::path::PathBuf;
use tracing_subscriber::{EnvFilter, fmt, prelude::*};

mod camera;
mod config;

use camera::Camera;
use config::Config;

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

impl Into<Config> for Cli {
    fn into(self) -> Config {
        Config {
            fd_model_path: self.fd_model_path,
            fr_model_path: self.fr_model_path,
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

    let window = String::from("Eyes That Blink");
    opencv::highgui::named_window(&window, opencv::highgui::WINDOW_AUTOSIZE)
        .expect("Failed to create window");

    let mut camera = Camera::new(0, Some(window));
    camera.start(&config);
}
