
function refresh()
{
	if (cueData)
	{
		var cueLength = document.getElementsByClassName("leftspot").length;
		var keys = Object.keys(cueData);
		var nearestNumber = keys.reduce(function(prev, curr) {
			return (Math.abs(curr - currentCue) < Math.abs(prev - currentCue) ? curr : prev);
		  });
		  
		var startIndex = keys.indexOf(nearestNumber.toString());

		// Calculate the start so the current cue is in the middle
		if (startIndex > Math.ceil(cueLength/2))
		{
			startIndex -= Math.ceil(cueLength/2);
		}
		else
		{
			startIndex = 0;
		}

		var leftNotes = document.getElementsByClassName("leftspot")
		var cueNumber = document.getElementsByClassName("midcue")
		var rightNotes = document.getElementsByClassName("rightspot")

		for (i = 0; i < cueLength; i++) { 
			presentCue = cueData[keys[startIndex+i]];

			leftNotes[i].innerHTML = presentCue['OP'].replace(/(?:\r\n|\r|\n)/g, '<br>')
			cueNumber[i].innerHTML = keys[startIndex+i]
			rightNotes[i].innerHTML = presentCue['P'].replace(/(?:\r\n|\r|\n)/g, '<br>')
		}

		
		document.getElementById('Cue').innerHTML = !!currentCue === true ? currentCue : "?" ;
	}
	else
	{
		document.getElementById('Cue').innerHTML = currentCue;
	}
}
