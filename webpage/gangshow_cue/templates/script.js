var input = {% raw data %}

console.log(input);

currentCue = 0;
standby = 0;

	function refresh() {
		currentScene = getScene(currentCue);
		nextScene = getScene(currentCue+1);
		document.getElementById('cue').innerHTML = !!currentCue === true ? currentCue : "?" 
		document.getElementById('current-scene').innerHTML = currentScene.Scene;
		document.getElementById('next-scene').innerHTML = nextScene.Scene;
		document.getElementById('current-notes-content').innerHTML = currentScene.Notes;
		document.getElementById('next-notes-content').innerHTML = nextScene.Notes;
	}

	function getScene(cueNumber) {
		foundScene = !!input[cueNumber] === true ? input[cueNumber].Scene : 'data loading error';
		foundNotes = !!input[cueNumber] === true ? input[cueNumber].Scene : 'data loading error';

		return {"Cue":cueNumber, 'Scene':foundScene, 'Notes':foundNotes};
	}

	function doConnect() {
		url = window.location.protocol == 'http:' ? 'ws://' : 'wss://';
		url += window.location.hostname + ":" + window.location.port + '/ws';
		websocket = new WebSocket(url);
		websocket.onopen = function(evt) { onOpen(evt) };
		websocket.onclose = function(evt) { onClose(evt) };
		websocket.onmessage = function(evt) { onMessage(evt) };
		websocket.onerror = function(evt) { onError(evt) };
	}

	function onOpen(evt) {
		document.getElementById('connected').className='green';
		// heartbeat()
	}

	function onClose(evt) {
		// Start reconection attepmts
		document.getElementById('connected').className='red';
		setTimeout(doConnect, 1000);
	}

	function onMessage(evt) {
		// proccess incoming message
		incomingMessage = JSON.parse(evt.data)
		if (incomingMessage.hasOwnProperty('cue'))
		{
			currentCue = incomingMessage['cue'];
			standby = incomingMessage['standby'];
			refresh();
		}
	}

	function onError(evt) {
		// writeToScreen('error: ' + evt.data + '\n');

		websocket.close();
		document.getElementById('connected').className='red';
		setTimeout(doConnect, 5000);
	}

	// function heartbeat() {
	// 	websocket.send('ping');
	// 	setTimeout(heartbeat, 1000);
	// }

doConnect();
window.addEventListener("load", refresh, false);
