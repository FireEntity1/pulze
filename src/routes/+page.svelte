<script>
  // @ts-nocheck

  import { invoke } from "@tauri-apps/api/core";
  import { onMount } from "svelte";

  let url = "";
  let message = "";
  let audioSrc = "";

  let audioSrcs = {};

  /**
   * @type {any[]}
   */
  let dir = [];

  onMount(async () => {
    dir = [...(await invoke("get_dir"))];
    for (const file of dir) {
      const base64 = await invoke("read_file_base64", { path: file });
      audioSrcs[file] = `data:audio/mp3;base64,${base64}`;
    }
      console.log(audioSrcs);
  });

  async function downloadAndPlay() {
    message = "Downloading...";
    const filePath = await invoke("download_video", { url });
    message = "Download complete! Loading audio...";
    const base64Data = await invoke("read_file_base64", { path: filePath });
    audioSrc = `data:audio/mp3;base64,${base64Data}`;
    message = "Ready to play!";
  }

  async function load(path) {
    console.log(path);
    const base64 = await invoke("read_file_base64", { path: path });
    return `data:audio/mp3;base64,${base64}`;
  }
</script>

<main>
  <h1>YouTube Downloader Test</h1>
  <input bind:value={url} placeholder="YouTube URL" />
  <button on:click={downloadAndPlay}>Download</button>
  <p>{message}</p>

  <ul>
    {#each dir as file}
      <li>{file}</li>
      <audio controls src={audioSrcs[file]}></audio>
    {/each}
  </ul>

  {#if audioSrc}
    <audio controls src={audioSrc}></audio>
  {/if}
</main>
