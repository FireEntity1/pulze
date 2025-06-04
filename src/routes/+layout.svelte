<script>
	import '../app.css';
	let { children } = $props();
	export const prerender = true
	export const ssr = false
</script>

{@render children()}
