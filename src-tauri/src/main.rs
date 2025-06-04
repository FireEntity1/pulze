#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::process::Command;
use std::fs;
use tauri::command;
use base64;

#[tauri::command]
fn download_video(url: String) -> Result<String, String> {
    let folder = dirs::document_dir().unwrap_or_else(|| std::path::PathBuf::from("."));
    let output_path = folder.join("pulze-audio.mp3");

    let output = std::process::Command::new("yt-dlp")
        .args([
            "-x",
            "--audio-format", "mp3",
            "-o",
            output_path.to_str().unwrap(),
            &url,
        ])
        .output();

    if let Ok(o) = output {
        if o.status.success() {
            return Ok(output_path.to_str().unwrap().to_string());
        } else {
            return Err(format!("Download failed:\n{}", String::from_utf8_lossy(&o.stderr)));
        }
    } else {
        return Err("Failed to start download process".into());
    }
}

#[command]
fn read_file_base64(path: String) -> Result<String, String> {
    fs::read(&path)
        .map(|bytes| base64::encode(&bytes))
        .map_err(|e| e.to_string())
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![download_video, read_file_base64])  // <-- Combine here!
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

