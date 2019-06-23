
function refresh() {
	if (cueData && cueData.hasOwnProperty(currentCue))
	{
		presentCue = cueData[currentCue];

		document.getElementById('Cue').innerHTML = !!currentCue === true ? currentCue : "" ;
		document.getElementById('Bracket').innerHTML = !!currentCue === true ? ")" : "" ;
		
		// only say scene not loaded if there's a problem with the data
		document.getElementById('NextSceneHeading').innerHTML = !!currentCue === true ? "Next Scene:" : "Scene not Loaded" ;
		for (prop in presentCue)
		{
			if(document.getElementById(prop))
			{
				document.getElementById(prop).innerHTML = presentCue[prop];
			}
			if (prop === "CastCall") {
				document.getElementById("CastCall").classList.remove("transition-class");
				setTimeout(function(){ 
					document.getElementById("CastCall").classList.add("transition-class");
				}, 100);

			}
		}
	}
	else
	{
		// document.getElementById('Cue').innerHTML = "?";
	}
}
