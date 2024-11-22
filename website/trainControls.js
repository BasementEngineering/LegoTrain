const joystick = document.getElementById('joystick');
const container = document.querySelector('.joystick-container');

var stopCallback = null;

function resetJoystickPosition() {
    joystick.style.top = `${container.offsetHeight - joystick.offsetHeight}px`;
}

var buttonIds = ["btn_direction_forwards","btn_direction_backwards"];//"stop_btn"];
var direction = 0;

function initButtons(){
    buttonIds.forEach(function(buttonId){
        document.getElementById(buttonId).addEventListener('click', function(){
            onDirectionButtonClicked(buttonId);
        });
    });

    document.getElementById("stop_btn").addEventListener('click', function(){
        if(stopCallback != null){
            stopCallback();
        }
        resetJoystickPosition();
    });

    onDirectionButtonClicked("btn_direction_forwards");
}

function setStopCallback(callback){
    stopCallback = callback;
}


function onDirectionButtonClicked(buttonId) {
    console.log('onDirectionButtonClicked');
    direction = buttonIds.indexOf(buttonId);
    buttonIds.forEach(function(id){
        if(id == buttonId){
            document.getElementById(id).style = 'background-color: var(--secondary-color)';
        }
        else{
            document.getElementById(id).style = 'background-color: var(--deactivated-color)';
        }
    });
}

function initTrainControls(){
    window.addEventListener('load', resetJoystickPosition);

    joystick.addEventListener('touchstart', (event) => {
        event.preventDefault();
        const onTouchMove = (event) => {
        updateJoystickPosition(event.touches[0]);
        //sendJoystickValue(getJoystickValue());
        };
    
        document.addEventListener('touchmove', onTouchMove);
        document.addEventListener('touchend', () => {
        document.removeEventListener('touchmove', onTouchMove);
        }, { once: true });
    });
    joystick.addEventListener('mousedown', () => {
        const onMouseMove = (event) => {
            updateJoystickPosition(event);
            //sendJoystickValue(getJoystickValue());
        };
    
        document.addEventListener('mousemove', onMouseMove);
        document.addEventListener('mouseup', () => {
            document.removeEventListener('mousemove', onMouseMove);
        }, { once: true });
    });

    initButtons();
}

function getSetpointValue(){
    var joystickValue = getJoystickValue();

    if(direction == 1){
        joystickValue = -joystickValue;
    }
     return joystickValue;   
}

function getJoystickValue() {
    const rect = container.getBoundingClientRect();
    const joystickRect = joystick.getBoundingClientRect();
    const centerY = rect.top + rect.height / 2;
    const joystickCenterY = joystickRect.top + joystickRect.height / 2;
    const maxDistance = rect.height / 2 - joystickRect.height / 2;
    const distance = joystickCenterY - centerY;
    return Math.round(51 -(distance / maxDistance) * 50);
}

function updateJoystickPosition(event) {
    const rect = container.getBoundingClientRect();
    const maxTop = rect.height - joystick.offsetHeight;
    let top = event.clientY - rect.top - joystick.offsetHeight / 2;
    top = Math.max(0, Math.min(maxTop, top));
    joystick.style.top = `${top}px`;
}

export {initTrainControls, getSetpointValue};