<script>
  import { invoke } from '@tauri-apps/api/core';
  
  let url = '';
  let message = '';
  let audioSrc = '';

  async function downloadAndPlay() {
    message = 'Downloading...';
    try {
      const filePath = await invoke('download_video', { url });
      message = 'Download complete! Loading audio...';

      const base64Data = await invoke('read_file_base64', { path: filePath });
      audioSrc = `data:audio/mp3;base64,${base64Data}`;
      message = 'Ready to play!';
    } catch (e) {
      message = `Error: ${e}`;
    }
  }
</script>

<h1>YouTube Downloader Test</h1>
<input bind:value={url} placeholder="YouTube URL" />
<button on:click={downloadAndPlay}>Download</button>
<p>{message}</p>

{#if audioSrc}
  <audio controls src={audioSrc}></audio>
{/if}
