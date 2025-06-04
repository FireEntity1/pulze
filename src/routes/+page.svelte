<script>
	import { onMount } from "svelte";
	// import { splitVendorChunk } from "vite";

	let player;

	onMount(() => {
		window.onYouTubeIframeAPIReady = function () {
			player = new YT.Player('player', {
				height: '0',
				width: '0',
				playerVars: {
					listType: 'playlist',
					list: '',
					autoplay: 1,
					controls: 0,
                    rel: 0,
                    disablekb: 1,
                    loop: 0,
				},
				events: {
					onReady: (event) => event.target.playVideo(),
					'onStateChange': onPlayerStateChange,
				}
			});
		};

		const script = document.createElement('script');
		script.src = "https://www.youtube.com/iframe_api";
		document.body.appendChild(script);

		setInterval(updateData, 500);
	});

	function onPlayerStateChange(event) {
		setTimeout(updateData, 400);
	}

	function next() {
		if (player && player.nextVideo) {
			player.nextVideo();
			setTimeout(updateData, 400);
		} else {
			console.error("player brokey");
		}
	}

    function previous() {
		if (player && player.nextVideo) {
			player.previousVideo();
			setTimeout(updateData, 400);
		} else {
			console.error("player brokey");
		}
	}

	function updateData() {
		let cover = "https://img.youtube.com/vi/" + player.getVideoData()['video_id'] + "/0.jpg";
		document.getElementById("cover").setAttribute('src', cover);

		document.getElementById("seek").setAttribute('max', player.getDuration())
	}

    function play() {

		if (player && player.nextVideo) {
            if (player.getPlayerState() == 1) {
                player.pauseVideo();
				updateData();
            } else if (player.getPlayerState() == 2 || player.getPlayerState() == -1) {
                player.playVideo();
				updateData();
            }
		} else {
			console.error("player brokey");
		}
	}

	function loadPlaylist() {
		event.preventDefault();
		let loaded = false;
		let url = document.getElementById("link").value;
		let splitUrl = url.split("&");
		for (let i = 0; i < splitUrl.length; i++) {
			if (splitUrl[i].includes("list=")) {
				let listId = splitUrl[i].replace("list=", "");
				player.loadPlaylist({list: listId});
				player.playVideo();
				loaded = true;
				setInterval(updateSeek, 1000);
			}
		}
		if (!loaded) {
			player.loadPlaylist({list: url})
			player.playVideo();
		}
	}

	function changeVolume(value) {
		player.setVolume(value);
	}

	function seek(value) {
		player.seekTo(value);
	}

	function updateSeek() {
		document.getElementById("seek").value = player.getCurrentTime();
	}
// https://www.youtube.com/watch?v=lc4nTM6M9KY&list=PLoogdHvYrJhlwTqkvGzds6BpT65nCBD28&index=1
</script> 

<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
<link
	href="https://fonts.googleapis.com/css2?family=Oxanium:wght@200..800&display=swap"
	rel="stylesheet"
/>

<div class="fixed inset-0 w-screen h-screen z-[-1] bg text-center items-center flex flex-col">
    <div class="m-2">
        <h1 class="font text-6xl justify-center" id="title">pulze</h1>
    </div>
	<form on:submit={loadPlaylist}>
		<input id="link">
	</form>

    <div id="player" class="justify-center flex"></div>
	<img id="cover" alt="cover" height="200px" width="400px">
    <div class="m-4">
        <button on:click={previous} class="font text-2xl bg-slate-900 p-7 rounded-3xl">Previous</button>
        <button on:click={next} class="font text-2xl bg-slate-900 p-7 rounded-3xl">Next</button>
        <button on:click={play} class="font text-2xl bg-slate-900 p-7 rounded-3xl">||</button>
		<input type="range" id="volume" min="0" max="100" on:change={() => changeVolume(document.getElementById("volume").value)}>
		<input type="range" id="seek" min="0" max="1" on:change={() => seek(document.getElementById("seek").value)}>
    </div>

</div>