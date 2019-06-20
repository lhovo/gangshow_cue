
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
