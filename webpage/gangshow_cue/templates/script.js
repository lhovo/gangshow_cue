var input = {% raw data %}

console.log(input);

currentCue = 23;

document.getElementById('current-scene').innerHTML = !!input[currentCue] === true ? input[currentCue].Scene : 'data loading error'; // enter current scene
document.getElementById('cue-number').innerHTML = currentCue // enter cue number
document.getElementById('next-scene').innerHTML = !!input[currentCue + 1] === true ? input[currentCue + 1].Scene : 'data loading error'; // enter next scene
