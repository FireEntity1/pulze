<script>
  // @ts-nocheck

  import { event, path } from "@tauri-apps/api";

  import { invoke } from "@tauri-apps/api/core";
  import { onMount } from "svelte";

  let url = "";
  let message = "";
  let audioSrc = "";

  let volume = 0.5;

  let playing = 0;

  let audioSrcs = {};

  let songname;
  $: songname = dir[playing] ? dir[playing].split("/")[dir[playing].split("/").length - 1] : "No file selected";

  /**
   * @type {any[]}
   */
  let dir = [];

  let audio;

  async function load_audio() {
    dir = [...(await invoke("get_dir"))];
    dir = dir.filter(
      (file) =>
        file.endsWith("ogg") || file.endsWith("mp3") || file.endsWith("webm"),
    );
    for (const file of dir) {
      console.log(file);
      const base64 = await invoke("read_file_base64", { path: file });
      audioSrcs[file] = `data:audio/mp3;base64,${base64}`;
    }
    songname = dir[playing] ? dir[playing].split("/")[dir[playing].split("/").length - 1] : "No file selected";
    console.log(audioSrcs);
  }

  onMount(async () => {
    load_audio();
    console.log(audioSrcs);
  });

  async function rename(file, new_file) {
    await invoke("rename_file", { file: file, newFile: new_file });

    console.log(file);
    console.log(new_file);
  }

  async function download() {
    message = "Downloading...";
    const filePath = await invoke("download_video", { url });
    message = "Download complete! Loading audio...";
    const base64Data = await invoke("read_file_base64", { path: filePath });
    message = "Loaded!";
    load_audio();
  }
</script>

<main class="text-center align-middle">
  <h1>PULZE</h1>
  <input bind:value={url} placeholder="YouTube URL" />
  <button onclick={download}>Download</button>
  <p>{message}</p>

  <form onsubmit={(event) => { event.preventDefault(); rename(dir[playing], event.target.elements.filename.value); }}>
  <input type="text" value={songname} name="filename">
</form>

  <p>{songname}</p>
  <audio bind:volume={volume} autoplay onended={(playing += 1)} controls src={audioSrcs[dir[playing]]}></audio>

  <input type="range" bind:value={volume} min="0" max="1" step="0.05">

  <button onclick={() => { playing = Math.max(0, playing - 1); }}> back </button>
  <button onclick={() => { playing = Math.min(dir.length - 1, playing + 1); }}> forward </button>
</main>
