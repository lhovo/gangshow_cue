var input = {% raw data %}

console.log(input);

currentCue = 23;

document.getElementById('current-scene').innerHTML = !!input[currentCue] === true ? input[currentCue].Scene : 'data loading error';
document.getElementById('cue-number').innerHTML = !!currentCue === true ? currentCue : "?" 
document.getElementById('next-scene').innerHTML = !!input[currentCue + 1] === true ? input[currentCue + 1].Scene : 'data loading error';
document.getElementById('current-notes-content').innerHTML = !!input[currentCue] === true ? input[currentCue].Notes : 'data loading error';
document.getElementById('next-notes-content').innerHTML = !!input[currentCue] === true ? input[currentCue + 1].Notes : 'data loading error';