use tracing_subscriber::{EnvFilter, fmt, prelude::*};

mod camera;
use camera::Camera;

fn main() {
    tracing_subscriber::registry()
        .with(fmt::layer())
        .with(EnvFilter::from_default_env())
        .init();

    let window = String::from("Eyes That Blink");
    opencv::highgui::named_window(&window, opencv::highgui::WINDOW_AUTOSIZE)
        .expect("Failed to create window");

    let mut camera = Camera::new(0, Some(window));
    camera.start();
}
