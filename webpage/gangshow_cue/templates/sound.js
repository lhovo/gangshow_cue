
function refresh()
{
	if (cueData && cueData.hasOwnProperty(currentCue*100))
	{
		var cueLength = document.getElementsByClassName("cueNumber").length;
		var keys = Object.keys(cueData);
		var startIndex = keys.indexOf(currentCue.toString());

		// Calculate the start so the current cue is in the middle
		if (startIndex > Math.ceil(cueLength/2))
		{
			startIndex -= Math.ceil(cueLength/2);
		}
		else
		{
			startIndex = 0;
		}

		var cueNumber = document.getElementsByClassName("cueNumber");
		var script = document.getElementsByClassName("script");
		var notes = document.getElementsByClassName("notes");

		for (i = 0; i < cueLength; i++) { 
			presentCue = cueData[keys[startIndex+i]];

			// Sound script has the cue numbers *100 to ensure non cue
			// lines are in order, lets bring that back to normal and divide here
			cueNumber[i].innerHTML = (keys[startIndex+i]/100).toFixed(2);
			script[i].innerHTML = presentCue['RunScript'].replace(/(?:\r\n|\r|\n)/g, '<br>');
			notes[i].innerHTML = presentCue['Notes'].replace(/(?:\r\n|\r|\n)/g, '<br>');
		}

		
		document.getElementById('Cue').innerHTML = !!currentCue === true ? currentCue : "?" ;
	}
	else
	{
		document.getElementById('Cue').innerHTML = currentCue;
	}
}
