var cueData = null;
var cueKeys = null;

currentCue = 1;
standby = 0;
websocket = null;

function refresh() {
	if (cueData && cueData.hasOwnProperty(currentCue))
	{
		presentCue = cueData[currentCue];
		document.getElementById('Cue').innerHTML = !!currentCue === true ? currentCue : "?" ;
		for (prop in presentCue)
		{
			if(document.getElementById(prop))
			{
				document.getElementById(prop).innerHTML = presentCue[prop];
			}
		}
	}
	else
	{
		document.getElementById('Cue').innerHTML = "?";
	}
}

function refresh_fullScript()
{
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

function getScene(cueNumber, findForward)
{
	if(cueData == null)
	{
		return {"Cue":'999', 'Scene':'Loading', 'Notes':'Loading'};
	}
	findpoint = cueNumber*1000;
	for (cuereturn = 0; (cueKeys[cuereturn] <= findpoint) && (cuereturn < cueKeys.length-1); cuereturn++)
	{
		//console.log(cueKeys[cuereturn])
	}
	if(cuereturn != 0) { cuereturn--; }
	retCue = cueKeys[cuereturn];
	retString = (retCue/1000).toString();
	foundScene = !!cueData[retCue] === true ? cueData[retCue].Scene : 'data loading error';
	if(foundScene.length == 0)
	{ 
		for (; cuereturn > 0; cuereturn--)
		{
			if(cueData[cueKeys[cuereturn].toString()].Scene.length !=0)
			{
				foundScene = cueData[cueKeys[cuereturn].toString()].Scene;
				break;
			}
		}
	}
	foundNotes = !!cueData[retCue] === true ? cueData[retCue].RunScript : 'data loading error';
	//console.log("requested " + cueNumber + " Found " + retCue);
	return {"Cue":retString, 'Scene':foundScene, 'Notes':foundNotes};
}

function doConnect()
{
	url = window.location.protocol == 'http:' ? 'ws://' : 'wss://';
	url += window.location.hostname + ":" + window.location.port + '/ws';
	websocket = null; // ensure we clean up the old connection
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

function onClose(evt)
{
	// Start reconection attepmts
	document.getElementById('connected').className='red';
	setTimeout(doConnect, 1000);
}

function onMessage(evt)
{
	// proccess incoming message
	incomingMessage = JSON.parse(evt.data)
	if (incomingMessage.hasOwnProperty('cue'))
	{
		currentCue = incomingMessage['cue'];
		standby = incomingMessage['standby'];
		refresh();
	}
}

function onError(evt)
{
	// writeToScreen('error: ' + evt.data + '\n');

	websocket.close();
	document.getElementById('connected').className='red';
	// setTimeout(doConnect, 5000);
}

function loadJSON(callback)
{   
	var xobj = new XMLHttpRequest();
	xobj.overrideMimeType("application/json");
	xobj.open('GET', '/script/'+document.getElementById("jsScript").getAttribute("lookup"), true);
	xobj.onreadystatechange = function ()
	{
		if (xobj.readyState == 4 && xobj.status == "200")
		{
			callback(xobj.responseText);
		}
	};
	xobj.send(null);  
 }

 function init()
 {
	loadJSON(function(response)
	{
		cueData = JSON.parse(response);
		cueKeys = Object.keys(cueData).map(Number);
		cueKeys.sort((a, b) => a - b);
		refresh();
	});
}

// function heartbeat() {
// 	websocket.send('ping');
// 	setTimeout(heartbeat, 1000);
// }

doConnect();
window.addEventListener("load", init, false);
