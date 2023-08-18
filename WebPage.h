#ifndef WebPage_h
#define WebPage_h

// Info about websockets
// https://javascript.info/websocket
const char WEBPAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
	<title>Remote Serial</title>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css">
	<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
	<script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.16.0/umd/popper.min.js"></script>
	<script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js"></script>
	<style>
		body{
			background: lightgray;
		}
		.flex-container {
			height: 100vh;
			background: black;
		}
		#content{
			overflow-y: auto;
			font-family: monospace;
			color: lightgreen;
		}
		#form-controls{
			background: darkgray;
		}
	</style>
</head>
<body>

<div class="container">
	<div class="d-flex align-items-start flex-column flex-container" style="">
		<div class="mb-auto p-2 w-100" id="content"></div>
		<div class="p-2 w-100" id="form-controls">
			<div class="form-row align-items-center">
				<div class="col-sm-1">
					<button id="btnClear" class="btn btn-secondary">Clear</button>
				</div>
				<div class="col-sm-1">
					<button id="btnConnection" class="btn btn-success">Connect</button>
				</div>
				<div class="col-sm-2">
					<div class="form-check text-center">
						<input type="checkbox" class="form-check-input connected-only" id="chkAutoScroll" checked>
						<label class="form-check-label connected-only" for="chkAutoScroll">Auto scroll</label>
					</div>
				</div>
				<div class="col-sm-7">
					<input type="text" id="txtCommand" class="form-control connected-only" placeholder="Enter command..." autocomplete="false">
				</div>
				<div class="col-sm-1">
					<button id="btnSend" class="btn btn-primary connected-only" disabled>Send</button>
				</div>
			</div>
		</div>
	</div>
</div>

<div class="modal fade" id="modal-message" tabindex="-1" role="dialog" aria-labelledby="form-title" aria-hidden="true">
	<div class="modal-dialog modal-dialog-centered" role="document">
		<div class="modal-content">
			<div class="modal-header">
				<h5 class="modal-title" id="form-title"></h5>
				<button type="button" class="close" data-dismiss="modal" aria-label="Close">
					<span aria-hidden="true">&times;</span>
				</button>
			</div>
			<div class="modal-body"></div>
			<div class="modal-footer">
				<button type="button" class="btn btn-secondary" data-dismiss="modal">Close</button>
			</div>
		</div>
	</div>
</div>

</body>

<script type="module">

	// Ansi colours
	const AnsiColour = {
		YELLOW:	"\x1B[0;93m",
		RED:	"\x1B[0;91m",
		GREEN:	"\x1B[0;92m"
	}

	// import ansi_up from CDN for displaying ansi formatting
	import { AnsiUp } from "https://cdn.jsdelivr.net/npm/ansi_up@6.0.0/ansi_up.min.js"

	// Ansi colour coding library
	const ansi_up = new AnsiUp();

	// Id for this session
	const sessionId = getSessionGuid();

	// IIFE for handling text animation
	const textAnimation = (function() {
		let timer;
		let frames = ["|", "/", "-", "\\"];
		let index = 0;
		
		function getFrame() {
			let frame = frames[index++];
			if (index == frames.length) {
				index = 0;
			}
			return frame;
		}

		function start(){
			var $span = $("<span>").attr("id", "connect-animation");
			$("#content").append($span);
			timer = setInterval(function(){
				$("#connect-animation").text(getFrame());
			}, 100);
		}

		function stop(){
			clearTimeout(timer);
			$("#connect-animation").remove();
		}

		return {
			Start: start,
			Stop: stop	
		}
	})();


	// IIFE for handing web socket connection
	const client = (function(){
		var url = "";
		var socket;

		function init(webSocketUrl){
			url = webSocketUrl;
		}

		function connect(){
			socket = new WebSocket(url);
			socket.onopen = client.OnConnect;
			socket.onmessage = client.OnMessage;
			socket.onclose = client.OnDisconnect;
			socket.onerror = client.OnError;
		}

		function disconnect(){
			socket.close();
		}

		function send(data){
			if (socket && socket.readyState == 1){
				socket.send(data);
				return true;
			}
			return false;
		}

		return {
			Init: init,
			Connect: connect,
			Disconnect: disconnect,
			Send: send,
			OnError: null,
			OnMessage: null,
			OnConnect: null,
			OnDisconnect: null
		}
	})();

	// Connect to host
	function connect(){

		client.OnConnect = function(e) {
			textAnimation.Stop();
			$("#btnConnection")
				.removeClass("btn-success")
				.addClass("btn-danger")
				.text("Disconnect")
				.data("connected", true)
				.attr("disabled", false);
			$(".connected-only").attr("disabled", false);
			outputMessage(`${AnsiColour.GREEN}${getTimeStamp()} Connected!\n`);
		}

		client.OnMessage = function(event) {
			outputMessage(event.data);
		}

		client.OnDisconnect = function(event) {
			textAnimation.Stop();

			$("#btnConnection")
				.removeClass("btn-danger")
				.addClass("btn-success")
				.text("Connect")
				.data("connected", false)
				.attr("disabled", false);
			$(".connected-only").attr("disabled", true);
			outputMessage(`${AnsiColour.YELLOW}${getTimeStamp()} Disconnected!\n`);
		}

		client.OnError = function(error) {
			const message = "There was a problem with the connection.";

			outputMessage(`${AnsiColour.RED}${getTimeStamp()} ${message}\n`);
			showPopup("Communication error", message);
		}

		outputMessage(`${AnsiColour.GREEN}${getTimeStamp()} Trying to connect...`);
		textAnimation.Start();
		outputMessage("\n");

		client.Connect();

		$("#btnConnection").attr("disabled", true);
	}

	// Output a message to the console div
	function outputMessage(message){
		const ansi = ansi_up.ansi_to_html(message).replace(/(?:\r\n|\r|\n)/g, "<br>");
		const $content = $("#content");
		
		$content.append(ansi);

		const autoScroll = $("#chkAutoScroll").is(":checked");
		if (autoScroll){
			$content.scrollTop($content.prop("scrollHeight"));
		}		
	}

	// Close socket
	function disconnect(){
		client.Disconnect();
	}

	// Send a message to the server
	function sendMessage(){
		const $textbox = $("#txtCommand");
		const data = $textbox.val();

		if (client.Send(data)){
			$textbox.val("");
		}
		else{
			console.log(`[send] could not send not connected!`);
			showPopup("Error sending message", "Could not send message because you are not connected");
		}
	}

	// Generate a guid useful for handling sessions
	// (an guid can be sent to the server to handle which client sent a message)
	function getSessionGuid() {
		return "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g, function(c) {
			const r = Math.random() * 16 | 0, v = c === "x" ? r : (r & 0x3 | 0x8);
			return v.toString(16);
		});
	}

	// Show a popup message
	function showPopup(title, body){
		$("#modal-message").modal("show");
		$(".modal-title").text(title);
		$(".modal-body").text(body);
		setTimeout(
			function(){
				$("#modal-message").fadeOut("slow", function(){ $("#modal-message").modal("hide"); });
			},
			5000
		);
	}

	// Generate a timestamp
	function getTimeStamp(){
		function padZeros(val){
			var str = val.toString();
			return (str.length < 2 ? "0" : "") + str;
		}
		var now = new Date(); 
		var ret = padZeros(now.getDate()) + "/" + padZeros(now.getMonth()+1)  + "/" + now.getFullYear() + " @ "  
		+ padZeros(now.getHours()) + ":" + padZeros(now.getMinutes()) + ":" + padZeros(now.getSeconds());
		return ret;
	}

	// Startup stuff
	$(function() {
		$("#btnSend").click(function(){
			sendMessage();
		});

		$("#txtCommand").on("keypress", function(e) {
			if (e.which == 13) {
				sendMessage();
			}
		});

		$("#btnConnection").click(function(){
			if ($(this).data("connected")){
				disconnect();
				return;
			}
			connect();
		});

		$("#btnClear").click(function(){
			$("#content span").remove();
		});

		$(".connected-only").attr("disabled", true);

		client.Init(`ws://${document.location.host}/remoteserialws`);

		$("#txtCommand").focus();

		connect();
	});
</script>
</html>
)rawliteral";

#endif