"use strict";

// Save a reference to the original. We'll be redefining it.
var parentDisconnected = connDisconnected;
var jitsiDomain, jitsiNickname, jitsiServerMuted, jitsiRoom, jitsiClientMuted, jitsiScriptLoaded, jitsiAPI, jitsiCurrentRoom, jitsiSilenced;
var jitsiMidMute, jitsiAddedAudioInputListener, jitsiLastAudioDevice, jitsiRoomPrefix;

function addJitsiComponentsTo(settingsDivName) {
    addComponent('audio_silence'  , settingsDivName, false, 'toggleSilenced', [], '<div class="button"><i id="silence_button" title="speaker shows whether you are playing audio chat" class="silenced fas fa-volume-mute"></i><span id="silence_indicator">Audio Chat</span></div>')
    addComponent('audio_mute'     , settingsDivName, false, 'toggleClientMuted', [], '<div class="button"><i id="mute_button" title="microphone shows whether you have muted yourself" class="muted fas fa-microphone-alt-slash"></i><span id="chat_indicator" title="text color shows whether server has muted you...">Mute/Unmute</span></div>');
    addComponent('audio_roomtext' , settingsDivName, false, false, [], '<div class="button">Room: <span id="audio_room"></span</div>');
    //addComponent('audio_device'   , settingsDivName , 'audio_device', false, [], '<select id="audio_input_devices"></div>');
}

function initJitsi() {
    printUnscreened("Note: starting with audio chat muted, make sure you can see the right sidebar for audio controls.", "connection debug");

    jitsiScriptLoaded = false;
    jitsiMidMute = false;
    jitsiAddedAudioInputListener = false;
    jitsiServerMuted = false;
    jitsiClientMuted = true;
    jitsiSilenced = false;
    jitsiRoom = "ChatLobby";
    jitsiNickname = loadCookie("user");

    if(window.location.hostname != "localhost") {
        var components = window.location.hostname.split(".");
        components[0] = "meet";
        jitsiDomain = components.join(".");
    } else {
        // No working local dev right now
    }

    // Show the current silenced and muted settings in the sidebar
    updateSilencedUI();
    updateMutedUI();

    // Add the Jitsi external_api script for our correct domain
    var script = document.createElement('script');
    script.type = 'text/javascript';
    script.src = 'https://' + jitsiDomain + '/external_api.js';
    document.head.appendChild(script);

    // If Jitsi fails for some reason, that should not block the
    // text-only connection from being established properly.
    script.onload = () => { jitsiScriptLoaded = true; jitsiReconnect() };
}

// This shouldn't be called until the Jitsi external API script loads
function jitsiReconnect() {
    // Script not loaded? Bail.
    if(!jitsiScriptLoaded) {
        return;
    }

    // Already have a connection? Kill it.
    if(jitsiAPI) {
        jitsiAPI.dispose();
        jitsiAPI = undefined;
        document.querySelector("#audio_room").textContent = "(none)";
    }

    // Disconnected? Then we're no longer effectively mid-mute.
    jitsiMidMute = false;

    // Want to be silent? Just don't reconnect.
    if(jitsiSilenced) {
        console.log("Reconnect: jitsi is silenced, so skip connection.");
        return;
    }

    // Jitsi Setup - for more Jitsi, see: https://jitsi.github.io/handbook/docs/dev-guide/dev-guide-iframe
    // For list of config options: https://github.com/jitsi/jitsi-meet/blob/master/config.js
    const options = {
        roomName: jitsiRoom,
        width: 200,
        height: 200,
        parentNode: document.querySelector('#meet'),
        configOverwrite: { startSilent: jitsiSilenced, startAudioOnly: true, startWithAudioMuted: jitsiClientMuted || jitsiServerMuted, fileRecordingsEnabled: false },
        userInfo: {
            displayName: jitsiNickname
        }
    };
    jitsiAPI = new JitsiMeetExternalAPI(jitsiDomain, options);
    jitsiCurrentRoom = jitsiRoom;
    document.querySelector("#audio_room").textContent = jitsiRoom;
    //jitsiAPI.getAvailableDevices().then(devices => updateAudioInputs(devices));
}

//// If you call this yourself, it messes things up. Please don't.
//function updateAudioInputs(devices) {
//    console.log("Updating audio input device list...");
//    var selector = document.querySelector('#audio_input_devices');
//    while(selector.options.length > 0) { selector.remove(0); }
//    devices.audioInput.forEach(dev => {
//        var opt = document.createElement('option');
//        opt.value = dev.label;
//        opt.title = dev.label;
//        if(dev.label.length > 10) {
//            opt.text = dev.label.slice(0,9) + "...";
//        } else {
//            opt.text = dev.label;
//        }
//        opt.deviceId = dev.deviceId;
//        selector.add(opt);
//        });
//    if(!jitsiAddedAudioInputListener) {
//        jitsiAddedAudioInputListener = true;
//        selector.addEventListener('change', (event) => {
//            console.log("Audio input device selector: changed");
//            if(selector.value != jitsiLastAudioDevice) {
//                console.log("Setting Jitsi audio input: " + selector.value)
//                jitsiAPI.setAudioInputDevice(selector.value);
//                jitsiLastAudioDevice = selector.value;
//            } else {
//                console.log("Audio device not changed from " + jitsiLastAudioDevice + ", not setting...");
//            }
//        });
//    }
//}

function toggleClientMuted() {
    jitsiClientMuted = !jitsiClientMuted;
    muteUnmute();
}

function toggleSilenced() {
    jitsiSilenced = !jitsiSilenced;

    updateSilencedUI();

    // Jitsi can't silence/unsilence after loading. So we just connect or not, basically.
    // It's also possible to keep a Jitsi connection but with no local output. But why?
    jitsiReconnect();
}

function updateSilencedUI() {
    let settingsDiv = document.querySelector("#audio_settings");
    let silenceButton = document.querySelector("#silence_button");
    if(jitsiSilenced) {
        settingsDiv.classList = "silenced";
        silenceButton.classList = "fas fa-volume-mute";
    } else {
        settingsDiv.classList = "audible";
        silenceButton.classList = "fas fa-volume-up";
    }
}

function updateMutedUI() {
    let muteButton = document.querySelector('#mute_button');
    if(jitsiClientMuted) {
        muteButton.classList = "muted fas fa-microphone-alt-slash";
    } else {
        muteButton.classList = "unmuted fas fa-microphone-alt";
    }

    let chatIndicator = document.querySelector("#chat_indicator");
    if(jitsiServerMuted) {
        chatIndicator.classList = "muted";
    } else {
        chatIndicator.classList = "unmuted";
    }
}

function muteUnmute() {
    updateMutedUI();

    if(jitsiSilenced) {
        // Silenced normally means disconnected, so don't tell Jitsi whether to mute.
        return;
    }

    if(!jitsiMidMute) {
        jitsiMidMute = true;
        jitsiAPI.isAudioMuted().then(muted => updateMuteState(muted));
    }
}

// If you call this yourself, it messes things up. Please don't.
function updateMuteState(muted) {
    jitsiMidMute = false;

    let shouldBeMuted = jitsiServerMuted || jitsiClientMuted;

    if(shouldBeMuted != muted) {
        jitsiAPI.executeCommand('toggleAudio');
    }
}

function isJitsiSkoot(num) {
    return (num >= 80 && num <= 82);
}

function doJitsiSkoot(num, msg) {
    switch(num) {
        case 80:
            // Set Jitsi room
            console.log("New Jitsi room: " + msg);
            if(msg != jitsiRoom) {
                jitsiRoom = msg;
                jitsiReconnect();
            } else {
                console.log("(Not changing room since it's the same.)");
            }
            break;

        case 81:
            // Set Jitsi server-muted
            if(msg != "true" && msg != "false") {
                badSkoot(num, msg);
            } else {
                console.log("Setting whether Jitsi is server-muted...");
                jitsiServerMuted = (msg == "true");
                muteUnmute();
            }
            break;

        case 82:
            // Set Jitsi display name
            console.log("Setting Jitsi display name:", msg);
            jitsiAPI.executeCommand("displayName", msg);
            break;

        default:
            badSkoot(num, msg);
            break;
    }
}

connDisconnected = function() {
    jitsiAPI.dispose();
    parentDisconnected();
}
