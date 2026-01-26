use tracing_subscriber::{EnvFilter, fmt, prelude::*};

mod camera;
use camera::Camera;

fn main() {
    tracing_subscriber::registry()
        .with(fmt::layer())
        .with(EnvFilter::from_default_env())
        .init();

    let mut camera = Camera::new();
    camera.start();
}
