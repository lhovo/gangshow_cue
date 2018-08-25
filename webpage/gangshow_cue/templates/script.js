var cueData = {% raw data %};
var cueKeys = Object.keys(cueData).map(Number);
cueKeys.sort((a, b) => a - b);

currentCue = 0;
standby = 0;

function refresh() {
	currentScene = getScene(currentCue, false);
	nextScene = getScene(currentCue+1, true);
	document.getElementById('cue').innerHTML = !!currentCue === true ? currentCue : "?" 
	document.getElementById('current-cue').innerHTML = currentScene.Cue;
	document.getElementById('next-cue').innerHTML = nextScene.Cue;
	document.getElementById('current-scene').innerHTML = currentScene.Scene;
	document.getElementById('next-scene').innerHTML = nextScene.Scene;
	document.getElementById('current-notes-content').innerHTML = currentScene.Notes;
	document.getElementById('next-notes-content').innerHTML = nextScene.Notes;
}

function getScene(cueNumber, findForward) {
	findpoint = findForward ? cueNumber : cueNumber+1;
	for (cuereturn = 0; (cueKeys[cuereturn] < findpoint) && (cuereturn < cueKeys.length-1); cuereturn++) {}
	retCue = findForward ? cueKeys[cuereturn] : cueKeys[cuereturn-1];
	foundScene = !!cueData[retCue] === true ? cueData[retCue].Scene : 'data loading error';
	foundNotes = !!cueData[retCue] === true ? cueData[retCue].Notes : 'data loading error';
	console.log("requested " + cueNumber + " Found " + retCue);
	return {"Cue":retCue, 'Scene':foundScene, 'Notes':foundNotes};
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
