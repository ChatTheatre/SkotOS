"use strict";
var conn, viewer, tree, debugtrack, gameCharacter, generic, hasChars, currentFile, currentFolder, removeFile, pageLoad, server, extra, web_protocol;
var c = {};
function initWebSocket(profile) {
	c = profile;
	if (conn) {
		writeToConsole('Closing pre-existing connection.');
		wsCloseIfOpen.close(1001, 'Closing previous connection.');
	}
	if (!window.WebSocket) {
		alert("<span style='color: red;'>ERROR:</span> Your browser does not support WebSockets, or has them currently disabled.", "connection error");
		return false;
	}

	if(profile.extra == "launcher") {
		generic = true;
	}

	extra = '';
	var woe_port = '';
	var http_port = '';

	if(profile.server) {
		server = profile.server;
	} else {
		server = getServer();
	}

	if(profile.web_protocol) {
		web_protocol = profile.web_protocol;
	} else {
		web_protocol = 'http';
	}

	if(profile.extra) {
		extra = profile.extra + '.';
	}

	if(profile.woe_port) {
		woe_port = profile.woe_port;
	} else {
		woe_port = getPort();
	}

	if(profile.http_port) {
		http_port = profile.http_port;
	}

	document.getElementById("view").src = web_protocol + "://" + extra + server + ":" + http_port + "/Dev/View.sam";
    
    console.log(document.getElementById("view").src);

    var wsuri = profile.protocol + "://" + server + ":" + woe_port;
    
    console.log(wsuri);
	
	try {
		conn = new WebSocket(wsuri);
	} catch (e) {
		if(e.name === "SecurityError") {
			alert("<span style='color: red;'>ERROR:</span> Your browser has blocked the connection according to its security rules.  This might be caused by opening the client in an usual way.  Please close this window and go to <a href='https://www.skotos.net/'>https://www.skotos.net/</a> to try again, or email <a href='mailto:ce@skotos.net'>ce@skotos.net</a> if this error persists.", "connection error");
		} else {
			alert("<span style='color: red;'>ERROR:</span> Your browser encountered an error while trying to connect.  Please close this window and go to <a href='https://www.skotos.net/'>https://www.skotos.net/</a> to try again, or email <a href='mailto:ce@skotos.net'>ce@skotos.net</a> with the information below if this error persists.<br><br><span style='color: grey;'>Additional error info:<br>"+safe_tags_replace(e.name)+"<br>"+safe_tags_replace(e.message)+"</span>", "connection error");
		}
		return false;
	}

	//conn.binaryType = 'arraybuffer';
	if (true) {
		addEvent(conn, 'open', onWSOpen);
		addEvent(conn, 'close', onWSClose);
		addEvent(conn, 'message', onWSMessage);
		addEvent(conn, 'error', onWSError);
	} else {
		conn.onopen    = function(evt) { onWSOpen(evt)    };
		conn.onclose   = function(evt) { onWSClose(evt)   };
		conn.onmessage = function(evt) { onWSMessage(evt) };
		conn.onerror   = function(evt) { onWSError(evt)   };
	}
}

function getServer() {
	return prompt("Server");
}

function getPort() {
	return prompt("Port");
}

function onWSOpen(evt) {
	window.onbeforeunload = function(evt) { wsCloseIfOpen(4001, "Window unloading."); };
	connConnected();
}
function onWSClose(evt) {
	connDisconnected();
}
function ab2str(buf) {
	return String.fromCharCode.apply(null, new Uint16Array(buf));
}
function onWSMessage(evt) {
	if (evt.data) {
		if(evt.data instanceof Blob) { 
			wsProcessBlob(evt.data);
		} else if(evt.data instanceof ArrayBuffer) {
			wsProcessArrayBuffer(evt.data);
		} else {
			wsProcessText(evt.data);
		}
	} else {
		debugMsg("<span style='color: blue;'>NO DATA IN RESPONSE</span> ", "received");
	}
}
function wsProcessBlob(blb) {
	var reader = new FileReader();
	addEvent(reader, "loadend", function() {
		doReceive(reader.result);
	});
	reader.readAsText(blb);
}
function wsProcessArrayBuffer(buf) {
	writeToConsole("buffer");
	doReceive(txt);
}
function wsProcessText(txt) {
	writeToConsole(txt);
	doReceive(txt);
}
function onWSError(evt) {
	console.log(evt);
	alert("<span style='color: red;'>WS ERROR:</span> The connection to the server encountered an error.  There may be an issue with your internet connection, or the game server may be restarting or temporarily offline.  You can wait a moment and try again, check your internet connection, or visit <a href='https://www.skotos.net/'>https://www.skotos.net/</a> and the forums to connfirm that the game server is up.  <br><br><span style='color: grey;'>Additional error info:<br>"+safe_tags_replace(evt.name)+"<br>"+safe_tags_replace(evt.message)+"</span>", "connection error");
}
function wsCloseIfOpen(code, message) {
	if (conn && conn.readyState < 2) {
		conn.close(code, message);
	}
}
//-----Generic Code
function debugMsg(text, format) {
	console.log(text);
	//printUnscreened(text, format + " debug");
}
function addEvent(obj, evType, fn, useCapture){
	if (obj.addEventListener){
		obj.addEventListener(evType, fn, useCapture);
	} else if (obj.attachEvent){
		obj.attachEvent("on"+evType, fn);
	} else {
		alert("Error attaching event: "+ obj + "; " + evType + "; " + fn + "; " + useCapture);
	}
}
function removeEvent(obj, evType, fn, useCapture) {
	if (obj.removeEventListener){
		obj.removeEventListener(evType, fn, useCapture);
	} else if (obj.detachEvent){
		obj.detachEvent("on"+evType, fn);
	} else {
		console.log("Error detaching event.");
	}
}
function cancelEvent(e) {
	e.noFixFocus = true;
	e.stopPropagation ? e.stopPropagation() : (e.cancelBubble=true);
}
if (!Array.prototype.indexOf) {
	//Defining this for IE8 and below
	Array.prototype.indexOf = function (obj, fromIndex) {
		if (fromIndex == null) {
			fromIndex = 0;
		} else if (fromIndex < 0) {
			fromIndex = Math.max(0, this.length + fromIndex);
		}
		for (var i = fromIndex, j = this.length; i < j; i++) {
			if (this[i] === obj)
				return i;
		}
		return -1;
	};
}
if ( !Date.prototype.toISOString ) {
	//Defining this for IE8 and below
	( function() {
		function pad(number) {
			var r = String(number);
			if ( r.length === 1 ) {
				r = '0' + r;
			}
			return r;
		}
		Date.prototype.toISOString = function() {
			return this.getUTCFullYear()
				+ '-' + pad( this.getUTCMonth() + 1 )
				+ '-' + pad( this.getUTCDate() )
				+ 'T' + pad( this.getUTCHours() )
				+ ':' + pad( this.getUTCMinutes() )
				+ ':' + pad( this.getUTCSeconds() )
				+ '.' + String( (this.getUTCMilliseconds()/1000).toFixed(3) ).slice( 2, 5 )
				+ 'Z';
		};
	}() );
}

function addComponent(newID, parentID, newClass, clickFunction, clickArgs, newContents, newTitle) {
	var parent = (typeof parentID == "object" ? parentID : document.getElementById(parentID));
	if (!parent) { console.log("Cannot find parent " + parentID + " for object " + newID + "... Skipping!"); return; }
	var newComponent = document.createElement('div');
	parent.appendChild(newComponent);
	if (newID) newComponent.id = newID;
	if (newClass) newComponent.className = newClass;
	if (newContents) newComponent.innerHTML = '<div id=\'' + newID + '-label\' class=\'label\'>' + newContents + '</div>';
	if (newTitle) newComponent.title = newTitle;
	if (newID) var newComponentLabel = document.getElementById(newID + '-label');
	if (newClass == 'subfolder' || newClass == 'file') {
		if(parent.outerHTML.includes('folder-open')) {
			//console.log("Parent: " + parent.id + " : Current File: " + currentFile);
			newComponent.style.display = "block";
			var $element = jQuery(parent).children("div.subfolder");
	    	currentFile = newID;
		} else {
			newComponent.style.display = "none";
		}
	}
	if (clickFunction) {
		var fun;
		if (typeof(clickFunction) == "string") {
			var def = clickFunction+'("'+clickArgs.join('","')+'")';
			fun = new Function(def);
		} else {
			fun = clickFunction;
			newComponentLabel.dataset.clickargs = clickArgs;
		}
		addEvent(newComponentLabel, "click", fun, {passive:true});
		newComponentLabel.style.cursor = "pointer";
	}

	return newComponent;
}

function connConnected() {
	conn.connected = true;
	conn.send("TreeOfWoe 1.0" + "\n");
    conn.send("SEND \n");
}

function connDisconnected() {
	conn.connected = false;
	console.log("DISCONNECTED", "connection");
	conn = null;
}

function isConnected() {
	return "";
}

//-----Interpreter Code
var receiveExtra = '';
var receiveMode = 'RAW';
var currentSubElements = [];
var receiveActive = false;
var loginDone = false;
function doReceive(msg) {
    console.log(msg);
    
	receiveActive = true;

	var port;

	if(c.http_port) {
		port = c.http_port;
	} else {
		port = 80;
	}

	var lines = msg.split('\r\n');

	for (var i=0; i<lines.length; i++) {
		var str = lines[i];

		if(str != undefined && str.includes("SET")) {
			var src = web_protocol + "://" + extra + server + ":" + port + str.split(/[ ,]+/)[1];
			var file = str.split(/[ ,]+/)[2];

			var folders = file.split(":");

			var currFolder = '';
			var lastFolder = '';
            var spaces = '';

			for(var j=0; j < folders.length-1; j++) {
				if(currFolder != '') {
					currFolder += ':' + folders[j];
				} else {
					currFolder = folders[j];
				}

				var temp = document.getElementById(currFolder);
				if (temp === null) {
					if(j == 0) {
						addComponent(currFolder, 'tree', 'folder', openFolder, [], spaces + '<i id=\'' + currFolder + '-icon\' class=\'far fa-folder\'></i>' + folders[j], currFolder);
					} else {
						addComponent(currFolder, lastFolder, 'subfolder', openFolder, [], spaces + '<i id=\'' + currFolder + '-icon\' class=\'far fa-folder\'></i>' + folders[j], lastFolder);
					}
				}

				lastFolder = currFolder;
				spaces += '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
			}

			if(!document.getElementById(file)) {
				addComponent(file, lastFolder, 'file', 'openFile', [src, file], spaces + '<i id=\'' + currFolder + '-icon\' class=\'fas fa-file\'></i>' + folders[j], src);
			}
			
		} else if(str.includes("CLEAR")) {
				document.getElementById(str.split(' ')[1]).remove();
		} 
	}

	receiveActive = false;
}

function isHidden(el) {
	return (el.offsetParent === null)
}

function openFolder() {
	var parent = document.getElementById(this.id.replace('-label', ''));
	if(currentFolder) document.getElementById(currentFolder + '-label').classList.remove("selected");
	if(currentFile) document.getElementById(currentFile).classList.remove("selected");
	currentFolder = parent.id;
	if(currentFolder) document.getElementById(currentFolder + '-label').classList.add("selected");
	var $element = jQuery(parent).children("div.subfolder");
   /* $element.sort(function(a, b) {
    	if(a.id < b.id) {
    		return -1;
    	} else {
    		return 1;
    	}
	}).appendTo(parent); */


	if(parent.classList.contains('folder') || parent.classList.contains('subfolder')) {
		var children = parent.childNodes;
		for(var i=1; i < children.length; i++) {
			if(isHidden(children[i])) {
				children[i].style.display = "block";
			} else {
				children[i].style.display = "none";
			}
		}

		var element = document.getElementById(parent.id + '-icon');
	    if(element.classList.contains('fa-folder-open')) {
			element.classList.remove("fa-folder-open");
			element.classList.add("fa-folder");
		} else {
			element.classList.remove("fa-folder");
			element.classList.add("fa-folder-open");
		} 
	}

	conn.send("SEND " + parent.id + "\n");
}

function isMobileDevice() {
    return (typeof window.orientation !== "undefined") || (navigator.userAgent.indexOf('IEMobile') !== -1);
};

function openFile(src, id) {
	if(currentFolder) document.getElementById(currentFolder + '-label').classList.remove("selected");
	currentFolder = null;
	var view = document.getElementById("view");
	var clone = view.cloneNode(true);
	clone.setAttribute('src', src);
	view.parentNode.replaceChild(clone, view);

	if(isMobileDevice()) {
		window.open(src, "_system");
	}

	if(currentFile) document.getElementById(currentFile).classList.remove("selected");
	currentFile = id;
	document.getElementById(currentFile).classList.add("selected");
}

function safe_tags_replace(str) {
	return typeof(str) === "string" ? str.replace(/[&<>]/g, replaceTag) : str;
}

//-----Initialization Code
addEvent(window, 'load', init, {once:true});

function init() {
	removeEvent(window, 'load', init, {once:true});
	tree = document.getElementById("tree");
	viewer  = document.getElementById("window");
	debugMsg("Client initialized and ready for connection.");
	initConnection();
	window.focus();
}
