//! Configuration parameters for the application.

use std::path::PathBuf;

#[derive(Debug, Clone)]
pub(crate) struct Config {
    pub score_threshold: f32,
    pub nms_threshold: f32,
    pub tok_k: u32,
}
