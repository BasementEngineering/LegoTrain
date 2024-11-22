var buttonIds = ["btn_mode1","btn_mode2","btn_mode3","btn_mode4"];
var choosenMode = 0;
var updateCallback = null;

function initModeSelctor(){
    console.log('initModeSelctor');
    buttonIds.forEach(function(buttonId){
        document.getElementById(buttonId).addEventListener('click', function(){
            onModeButtonClicked(buttonId);
        });
    });

    setChoosenMode(0);
}

function onModeButtonClicked(buttonId) {
    console.log('onModeButtonClicked');
    choosenMode = buttonIds.indexOf(buttonId);
    buttonIds.forEach(function(id){

        if(updateCallback != null){
            updateCallback(choosenMode);
        }

        if(id == buttonId){
            console.log("Secondary: " + id);
            document.getElementById(id).style = 'background-color: var(--secondary-color)';
        }
        else{
            console.log("Deactivated: " + id);
        document.getElementById(id).style = 'background-color: var(--deactivated-color)';
        }
    });
}

function getChoosenMode(){
    return choosenMode;
}

function setChoosenMode(mode){
    onModeButtonClicked(buttonIds[mode]);
}

function setUpdateCallback(callback){
    updateCallback = callback;
}

export {initModeSelctor, getChoosenMode, setChoosenMode,setUpdateCallback};