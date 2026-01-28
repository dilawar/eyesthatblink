use opencv::core::Mat;

pub fn show_frame(frame: &Mat) -> anyhow::Result<()> {
    let window = String::from("Eyes That Blink");
    opencv::highgui::imshow(&window, frame)?;
    let key = opencv::highgui::wait_key(1)?;
    if key > 0 && key != 255 {
        println!("Key pressed: {}", key);
        std::process::exit(0);
    }
    Ok(())
}
