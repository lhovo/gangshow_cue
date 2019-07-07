
function refresh() {
	if (cueData && cueData.hasOwnProperty(currentCue))
	{
		presentCue = cueData[currentCue];

		document.getElementById('Cue').innerHTML = !!currentCue === true ? currentCue : "" ;
		document.getElementById('Bracket').innerHTML = !!currentCue === true ? ")" : "" ;
		
		// only say scene not loaded if there's a problem with the data
		document.getElementById('NextSceneHeading').innerHTML = !!currentCue === true ? "Next Scene:" : "Scene not Loaded" ;
		var hasChanged = false;
		for (prop in presentCue)
		{
			if(document.getElementById(prop))
			{
				var presentValue = document.getElementById(prop).innerHTML;
				document.getElementById(prop).innerHTML = presentCue[prop].replace(/(?:\r\n|\r|\n)/g, '<br>');
				hasChanged = hasChanged | presentValue != document.getElementById(prop).innerHTML;
			}
		}
		if (hasChanged) {
			document.getElementById("CastCall").classList.remove("transition-class");
			setTimeout(function(){ 
				document.getElementById("CastCall").classList.add("transition-class");
			}, 100);
		}
	}
	else
	{
		// document.getElementById('Cue').innerHTML = "?";
	}
}
