<!DOCTYPE html>

<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
<link
    href="https://fonts.googleapis.com/css2?family=Oxanium:wght@200..800&display=swap"
    rel="stylesheet"
/>

<html lang="en" class="bg text-6xl text-center">
    <body>
        <div class="m-2">
            <h1 class="font">pulze</h1>
        </div>
        <div id="player"></div>
        <!-- <button on:click={next}>Play</button> -->

        <script>
            const script = document.createElement('script');
            script.src = "https://youtube.com/iframe_api";
            document.body.appendChild(script);

            let player;
            
            function onYouTubeIframeAPIReady() {
                player = new YT.Player('player', {
                    height: '360',
                    width: '640',
                    playerVars: {
                        listType: 'playlist',
                        list: 'RDsF80I-TQiW0',
                        autoplay: 1,
                        controls: 0
                    },
                    events: {
                        onReady: (event) => event.target.playVideo()
                    }
                });
            }

            function next() {
                if (player && player.playVideo) {
                    player.nextVideo();
                } else {
                    console.error("Player is not ready yet.");
                }
            }
        </script>
    </body>
</html>
