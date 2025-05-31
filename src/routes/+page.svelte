<script>
	import { onMount } from "svelte";

	let player;

	onMount(() => {
		window.onYouTubeIframeAPIReady = function () {
			player = new YT.Player('player', {
				height: '360',
				width: '640',
				playerVars: {
					listType: 'playlist',
					list: 'RDsF80I-TQiW0',
					autoplay: 1,
					controls: 0,
                    rel: 0,
                    disablekb: 1,
                    loop: 0,
				},
				events: {
					onReady: (event) => event.target.playVideo(),
				}
			});
		};

		const script = document.createElement('script');
		script.src = "https://www.youtube.com/iframe_api";
		document.body.appendChild(script);
	});

	function next() {
		if (player && player.nextVideo) {
			player.nextVideo();
		} else {
			console.error("player brokey");
		}
	}

    function previous() {
		if (player && player.nextVideo) {
			player.previousVideo();
		} else {
			console.error("player brokey");
		}
	}
</script>

<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
<link
	href="https://fonts.googleapis.com/css2?family=Oxanium:wght@200..800&display=swap"
	rel="stylesheet"
/>

<div class="fixed inset-0 w-screen h-screen z-[-1] bg text-center items-center flex flex-col">

    <div class="m-2">
        <h1 class="font text-6xl justify-center">pulze</h1>
    </div>
    <div id="player" class="justify-center flex"></div>
    <div class="m-4">
        <button on:click={previous} class="font text-2xl bg-slate-900 p-7 rounded-3xl">Previous</button>
        <button on:click={next} class="font text-2xl bg-slate-900 p-7 rounded-3xl">Next</button>
    </div>

</div>