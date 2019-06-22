
function refresh() {
	if (cueData && cueData.hasOwnProperty(currentCue))
	{
		presentCue = cueData[currentCue];

		document.getElementById('Cue').innerHTML = !!currentCue === true ? currentCue : "" ;
		document.getElementById('bracket').innerHTML = !!currentCue === true ? ")" : "" ;
		
		// only say scene not loaded if there's a problem with the data
		document.getElementById('next-scene').innerHTML = !!currentCue === true ? "Next Scene:" : "Scene not Loaded" ;
		for (prop in presentCue)
		{
			if(document.getElementById(prop))
			{
				document.getElementById(prop).innerHTML = presentCue[prop];
			}
			if (prop === "CastCall") {
				document.getElementById("CastCall").classList += " transition-class";
			}
		}
	}
	else
	{
		// document.getElementById('Cue').innerHTML = "?";
	}
}
