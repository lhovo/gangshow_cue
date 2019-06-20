
function refresh()
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
