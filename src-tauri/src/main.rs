#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use base64;
use std::fs::{self, ReadDir};
use std::process::Command;
use tauri::command;

#[tauri::command]
fn download_video(url: String) -> Result<String, String> {
    let folder = dirs::document_dir().unwrap().join("pulze");
    fs::create_dir_all(&folder).unwrap();

    let title = Command::new("yt-dlp")
        .args(["--get-title", &url])
        .output()
        .unwrap();
    let title_string = String::from_utf8_lossy(&title.stdout).trim().to_string();


    let output_path = folder.join(title_string);

    let output = std::process::Command::new("yt-dlp")
        .args([
            "-x",
            "--audio-format",
            "mp3",
            "-o",
            folder.join("%(title)s.%(ext)s").to_str().unwrap(),
            &url,
        ])
        .output();

    if let Ok(o) = output {
        if o.status.success() {
            return Ok(output_path.to_str().unwrap().to_string());
        } else {
            return Err(format!(
                "Download failed:\n{}",
                String::from_utf8_lossy(&o.stderr)
            ));
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

#[command]
fn get_dir() -> Vec<String> {
    let folder = dirs::document_dir().unwrap().join("pulze");
    fs::create_dir_all(&folder).unwrap();

    let path = folder.clone();

    let file_list: Vec<String> = fs::read_dir(folder)
        .unwrap()
        .map(|entry| entry.unwrap().path().display().to_string())
        .collect();

    println!("Returning file list: {:?}", file_list);

    file_list
}

#[command]
fn rename_file(file: String, new_file: String) -> Result<(), String> {
    let folder = dirs::document_dir().unwrap().join("pulze");
    println!("Renaming file in folder: {:?}", folder);

    let old_path = folder.join(file);
    let new_path = folder.join(new_file);

    fs::rename(&old_path, &new_path).map_err(|e| e.to_string())
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            download_video,
            read_file_base64,
            get_dir,
            rename_file
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
