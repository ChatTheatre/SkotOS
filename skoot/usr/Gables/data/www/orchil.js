"use strict";
var conn, output, input, debugtrack, gameCharacter, generic, hasChars, http_port, skotosAttributes;
var macros;
var c = {};
//-----Protocol Code
	function initAJAX(profile) {
		conn = {
			queue: [],
			busy: false,
			send: function (message) {
				this.queue.push(message);
				this.push();
			},
			push: function () {
				if (this.busy != false) {
					return false;
				}
				if (this.queue.length < 1) {
					return false;
				}
				this.busy = this.queue[0];
				this.queue.shift();
				var x;
				if (window.XMLHttpRequest) { //Normal browsers
					x=new XMLHttpRequest();
				} else { //IE5/6
					x=new ActiveXObject('Microsoft.XMLHTTP');
				}
				x.onreadystatechange=function() {
					if (x.readyState==4) {
						if (x.status==200) {
							debugMsg("<span style='color: blue;'>SAFE RESPONSE:</span> " + safe_tags_replace(x.responseText), "received");
							doReceive(x.responseText);
							conn.busy = false;
							conn.push();
						} else {
							printScreened("<span style='color: red;'>ERROR:</span> HTTP code " + x.status, "connection error");
						}
					}
				}
				x.open("POST",profile.path, true);
				x.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
				x.send("m="+this.busy);
				return this.busy;
			}
		};
		printScreened("AJAX connection initialized.", "connection");
	}
	function initWebSocket(profile) {
		c = profile;
		if (conn) {
			writeToConsole('Closing pre-existing connection.');
			wsCloseIfOpen.close(1001, 'Closing previous connection.');
		}
		if (!window.WebSocket) {
			printScreened("<span style='color: red;'>ERROR:</span> Your browser does not support WebSockets, or has them currently disabled.", "connection error");
			return false;
		}

		if(profile.extra == "launcher") {
			generic = true;
		}

		if(profile.chars == true) {
			hasChars = true;
		}
        
        if(profile.http_port) http_port = profile.http_port;
		
		var wsuri = profile.protocol + "://" + profile.server + ":" + profile.port + profile.path;

		var wsuri = profile.protocol + "://" + profile.server + ":" + profile.port + profile.path;
		printUnscreened("Starting connection to: " + wsuri, "connection debug");
		try {
			conn = new WebSocket(wsuri);
		} catch (e) {
			if(e.name === "SecurityError") {
				printScreened("<span style='color: red;'>ERROR:</span> Your browser has blocked the connection according to its security rules.  This might be caused by opening the client in an usual way.  Please close this window and go to <a href='https://www.skotos.net/'>https://www.skotos.net/</a> to try again, or email <a href='mailto:ce@skotos.net'>ce@skotos.net</a> if this error persists.", "connection error");
			} else {
				printScreened("<span style='color: red;'>ERROR:</span> Your browser encountered an error while trying to connect.  Please close this window and go to <a href='https://www.skotos.net/'>https://www.skotos.net/</a> to try again, or email <a href='mailto:ce@skotos.net'>ce@skotos.net</a> with the information below if this error persists.<br><br><span style='color: grey;'>Additional error info:<br>"+safe_tags_replace(e.name)+"<br>"+safe_tags_replace(e.message)+"</span>", "connection error");
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
		printScreened("<span style='color: red;'>WS ERROR:</span> The connection to the server encountered an error.  There may be an issue with your internet connection, or the game server may be restarting or temporarily offline.  You can wait a moment and try again, check your internet connection, or visit <a href='https://www.skotos.net/'>https://www.skotos.net/</a> and the forums to connfirm that the game server is up.  <br><br><span style='color: grey;'>Additional error info:<br>"+safe_tags_replace(evt.name)+"<br>"+safe_tags_replace(evt.message)+"</span>", "connection error");
	}
	function wsCloseIfOpen(code, message) {
		if (conn && conn.readyState < 2) {
			conn.close(code, message);
		}
	}
//-----Generic Code
	var tagsToReplace = {"&":"&amp;", "<":"&lt;", ">":"&gt;"};
	function replaceTag(tag) {
		return tagsToReplace[tag] || tag;
	}
	function safe_tags_replace(str) {
		return typeof(str) === "string" ? str.replace(/[&<>]/g, replaceTag) : str;
	}
	function replaceBadMSCharacters(text) {
		return text.replace(/[\u2018\u2019\u201A]/g, "\'").replace(/[\u201C\u201D\u201E]/g, "\"").replace(/\u2026/g, "...").replace(/[\u2013\u2014]/g, "-").replace(/\u02C6/g, "^").replace(/\u2039/g, "<").replace(/\u203A/g, ">").replace(/[\u02DC\u00A0]/g, " ");
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
	function debugMsg(text, format) {
		console.log(text);
		//printUnscreened(text, format + " debug");
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
	function addClass(node, cls) {
		writeToConsole("+ old: "+node.ClassName+" adding: "+cls);
		node.className = (node.ClassName?node.ClassName:"") + " " + cls;
		writeToConsole("+ new: "+node.ClassName);
	}
	function removeClass(node, cls) {
		writeToConsole("- old: "+node.ClassName+" removing: "+cls);
		if(node && node.className && node.className.indexOf(cls) >= 0) {
			var pattern = new RegExp('\\s*' + cls + '\\s*');
			node.className = node.className.replace(pattern, ' ');
		}
		writeToConsole("- new: "+node.ClassName);
	}
//-----UI Code
	//-----Prefs and Init
		var prefs;
		var pref_options = {
			arrow_behavior: {
				cat: "control",
				type: ["options"],
				desc: "How the up/down arrow keys behave when the input box already has text.",
				def: "always_scroll",
				opt: {
					always_scroll: "Pressing the Up or Down arrow keys by themselves will always scroll through your command history. (This is the behavior of the Alice client.)",
					current_first: "Pressing the Up or Down arrow keys will take you to the beginnging or end of the current line; or, if you are already there, then scroll through your command history.  The Ctrl key will force it to always scroll through your command history.  (This is the behavior of the Zealotry client.)"
				}
			},
			horizontal_scroll: {
				cat: "layout",
				type: ["options"],
				desc: "Management of the horizontal scroll bar for the output window.",
				def: "normal",
				opt: {
					normal: "The horizontal scroll bar is visible if you have any extra-long lines in your scrollback.",
					force_wrap: "Extra long-lines are forced to wrap, even if it breaks formatting.  Prevents horizontal scrollbars.",
					force_chop: "Extra-long lines have the extra length chopped off (except in logs).  Probably not a good idea.  Prevents horizontal scrollbars.",
					automatic: "The horizontal scroll bar is visible if your CURRENT view has any extra-long lines.  (Except when scrolling up, in which case it is visible if anything in your scrollback needs it.)",
					full_adaptive: "Same as automatic, but keeps working when you scroll up.  May slow down client.  Experimental.  Not yet implemented."
				},
				onChange: function(old) {
					var wrap = "pre";
					var flow = "auto";
					switch (prefs.horizontal_scroll) {
						case "normal":
							wrap = "pre";
							flow = "auto";
							break;
						case "force_wrap":
							wrap = "pre-wrap";
							flow = "auto";
							break;
						case "force_chop":
							wrap = "pre";
							flow = "hidden";
							//make output not have scrollbars
							break;
						case "automatic":
							wrap = "pre";
							flow = scrolledToBottom ? "hidden" : "auto";
							break;
						case "full_adaptive":
							wrap = "pre";
							flow = "hidden";
							break;
					}
					wrap = "#output pre {white-space:" + wrap + ";}";
					flow = "#output {overflow-x: " + flow + ";}";
					cssMods.pre_wrap = setStyle(wrap+" "+flow,cssMods.pre_wrap);
					if (old==="automatic") {
						removeEvent(window, "resize", updateAutomaticScroll);
						removeEvent(output, "scroll", updateAutomaticScroll);
					} else if (old==="full_adaptive") {
						removeEvent(window, "resize", updateAdaptiveScroll);
						removeEvent(output, "scroll", updateAdaptiveScroll);
					}
					if (prefs.horizontal_scroll==="automatic") {
						currentlyAdjustingScroll = false;
						currentAdjustingMode = false;
						updateAutomaticScroll();
						addEvent(window, "resize", updateAutomaticScroll, {passive:true});
						addEvent(output, "scroll", updateAutomaticScroll, {passive:true});
					} else if (prefs.horizontal_scroll==="full_adaptive") {
						currentlyAdjustingScroll = false;
						currentAdjustingMode = false;
						updateAdaptiveScroll();
						addEvent(window, "resize", updateAdaptiveScroll, {passive:true});
						addEvent(output, "scroll", updateAdaptiveScroll, {passive:true});
					}
				}
			},
			continuous_animations: {
				cat: "styling",
				type: ["options"],
				desc: "What version to play of animations that stay for a while.",
				def: "subtle_only",
				dis: true,
				opt: {
					subtle_only: "Don't play strong/distracting versions of continuous animations, but do play subtle versions when available.",
					subtle_preferred: "Play the subtle versions of continuous animated effects when available, or stronger versions if that'all that's available.",
					bold_preferred: "Play the strongest, most animated and/or most distracting versions of continuous animations available.",
					none: "Do not play continuous animated effects."
				}
			},
			brief_animations: {
				cat: "styling",
				type: ["options"],
				desc: "What version to play of animations that only appear for a moment.",
				def: "subtle_preferred",
				dis: true,
				opt: {
					subtle_only: "Don't play strong/distracting versions of brief animations, but do play subtle versions when available.",
					subtle_preferred: "Play the subtle versions of brief animated effects when available, or stronger versions if that'all that's available.",
					bold_preferred: "Play the strongest, most animated and/or most distracting versions of brief animations available.",
					none: "Do not play brief animated effects."
				}
			},
			command_history_lines: {
				cat: "control",
				type: ["integer", "nonnegative"],
				desc: "Number of recent commands to remember for re-entry.  High numbers may affect client lag.",
				on0: "No limit",
				def: 100,
				onChange: function(old) { while(prefs.command_history_lines && commandHistory.length > prefs.command_history_lines) commandHistory.shift(); if (historyPosition > commandHistory.length) historyPosition = commandHistory.length;}
			},
			output_click: {
				cat: "control",
				type: ["options"],
				desc: "Whether to snap the cursor to the input box when you click the output area.",
				def: "focus_input",
				opt: {
					focus_input: "When the output area (scrollback) is clicked, keep the focus on the input box, so you can keep typing without needing to click the input box.  Selecting text in the output area overrides this, putting focus on the output area (so that the text can be copied, etc.)",
					focus_output: "When the output area (scrollback) is clicked, move focus to the output area.  This disables typing until the input box is clicked or the tab key is pressed, but may make the text easier to interact with in some browsers."
				}
			},
			scrollback_count: {
				cat: "layout",
				type: ["integer", "nonnegative"],
				desc: "Number of 'elements' (lines) to keep in scrollback.  Very high numbers may affect client lag after extended play.",
				def: 0,
				on0: "No limit",
				onChange: function(old) { while(prefs.scrollback_count && output.childNodes.length > prefs.scrollback_count) output.removeChild(output.firstChild);}
			},
			local_echo: {
				cat: "layout",
				type: ["options"],
				desc: "Whether to show the commands you type, in the output area after you send them.",
				def: "on",
				opt: {
					on: "Commands you type appear in the output area after you send them.",
					off: "Commands you type are not shown at all after you send them.",
					temporary: "Commands you type are shown after sending them, until something new is received from the server.  This makes the command visible if there is lag, but replaces it with the result once it arrives."
				}
			},
			echo_color: {
				cat: "styling",
				type: ["color", "noquotes"],
				desc: "A custom color for the local echo - your commands as they appear in the output window (if they are visible).  Color names or hex codes may be used.",
				def: "",
				onChange: function(old) {
					var x = prefs.echo_color.replace(/['"]+/g,'');
					if (x) {
						x = "#output .inputecho {color:" + x+";}";
						cssMods.echo_color = setStyle(x, cssMods.echo_color);
					}
				}
			},
			background_color: {
				cat: "styling",
				type: ["color", "noquotes"],
				desc: "A custom background color.  Color names or hex codes may be used. Set 'none' to clear.",
				def: "",
				onChange: function(old) {
					var x = prefs.background_color.replace(/['"]+/g, '');
					if (x) {
						x = "#output { background-color:" + x + " !important;}"
						cssMods.background_color = setStyle(x, cssMods.background_color);
					}
				}
			},
			theme: {
				cat: "styling",
				type: ["options"],
				desc: "The colors and styles used for text.",
				def: "skotos",
				opt: {
					light: "A light background wtih dark text.  The standard Skotos appearance.",
					dark: "A dark background with light text.  Considered by many to be easier on the eyes.",
					dark_hc: "A high-contrast variant of the dark theme.",
					terminal: "Old style - everything in small, monospace fonts with a dark background.  Doesn't work well with most font_face settings.",
					skotos: "Assume the server will set the theme"
				},
				onChange: function(old) { setActiveStyleSheet(prefs.theme); }
			},
			font_face: {
				cat: "styling",
				type: ["string", "noquotes"],
				desc: "The custom font of the main text area.  Can be any font installed on your system and available to your browser, but must be spelled correctly and the exact name it is installed as.",
				def: "",
				onChange: function(old) {
					var x = prefs.font_face.replace(/['"]+/g,'');
					if (x) x = "#core {font-family:\"" + x + "\", minionpro-regular, \"Open Sans\", Roboto, Lato, Verdana, sans-serif;}"
					cssMods.font_face = setStyle(x, cssMods.font_face);
				}
			},
			keep_last_command: {
				cat: "control",
				type: ["options"],
				desc: "What the input area does after you send a command.",
				def: "off",
				opt: {
					off: "After sending a command, the input area is cleared, ready for the next command.",
					on:  "After sending a command, that command is kept in the input area, highlighted.  As a result, pressing Enter again repeats that command.  If you just start typing instead, the previous command is replaced with what you type."
				}
			},
			highlight_to_copy: {
				cat: "control",
				type: ["options"],
				desc: "Automatically copies highlighted text in the output area to the clipboard (similar to a unix terminal).  Not supported by all browsers.",
				def: "off",
				opt: {
					off: "Highlighting text in the output window does nothing.",
					on:  "Highlighting text in the output window copies it to the clipboard."
				},
				onChange: function(old) {
					if (prefs.highlight_to_copy=="on") {
						addEvent(output, "mouseup", copyOutputSelection, {passive:true});
					} else {
						removeEvent(output, "mouseup", copyOutputSelection, {passive:true});
					}
				}
			},
			autolog: {
				cat: "system",
				type: ["options"],
				desc: "Automatically start saving log files when the client starts.",
				def: "off",
				dis: true,
				opt: {
					off: "Do not automatically save logs.",
					localstorage:  "Automatically save logs in the browser's local storage.  Note that the browser will automatically delete these logs at certain events (such as clearing cache/history, or when memory is needed on mobile devices), so they should be regularly backed up elsewhere.",
					filesystem: "Automatically save logs to the filesystem (usually hard drive or equivalent).  Requires some sort of addon/extension/plugin to enable, but when available, is less likely to run into space/deltion issues than localstorage.  NOT YET ENABLED."
				}
			},
			prompt_savelog: {
				cat: "system",
				type: ["options"],
				desc: "Automatically prompt to save a log upon disconnect.  (Does not work if the client is refreshed/closed without properly disconnecting; be sure to use EXIT or similar commands to disconnect first.)",
				def: "off",
				opt: {
					off: "Do not automatically prompt to save logs.",
					on:  "Prompt to save log on disconnect"
				}
			},
			logging_path: {
				cat: "system",
				type: ["string"],
				desc: "For logs saved to the filesystem, this is the directory the logs are saved in.  NOTE: FILESYSTEM LOGGING NOT YET ENABLED.",
				def: "",
				dis: true,
			},
			logging_format: {
				cat: "system",
				type: ["options"],
				desc: "Format to save logs in.  HTML preserves some formatting better.",
				def: "html",
				opt: {
					html:  "Save the logs as non-styled HTML, with layout preserved.  These are best viewed in a browser.",
					plain: "Convert logs to plaintext.  These lose some layout information, but can be viewed in any text editor."
				}
			},
			command_history_min: {
				cat: "control",
				type: ["integer", "nonnegative"],
				desc: "The minimum length a line must be to be saved in the command history.  Setting it to 2, for example, lets it ignore n, s, e, w, etc., so that more important lines are kept accessible.",
				def: 1
			},
			text_size: {
				cat: "styling",
				type: ["integer", "nonnegative"],
				desc: "The size (in px) of the font used for the main display.  Common values are 10-24; most browsers default to 14 or 16.  Note that most browsers have a built-in way to adjust the size of the entire client, usually with key combinations like Ctrl++, Ctrl+- and Ctrl+0.",
				def: 0,
				on0: "Your browser's default",
				onChange: function(old) {var x = ((prefs.text_size > 7 && prefs.text_size < 501)?"#core {font-size:"+prefs.text_size+"px;}":""); cssMods.text_size = setStyle(x,cssMods.text_size);}
			},
			input_minheight: {
				cat: "layout",
				type: ["integer", "nonnegative"],
				desc: "The default size (in lines) of the area used for input.  Will automatically cap at half the client height.",
				def: Math.max(2, 1 + Math.ceil((window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight) / (window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth))),
				onChange: function(old) {recalcInputLineHeight(true);}
			},
			hotkeys_enabled: {
				cat: "control",
				type: ["options"],
				desc: "Whether to have hotkeys (keys that immediately do something when pressed) enabled.  These are used primarily for numpad navigation without needing to press Enter.",
				def: "off",
				opt: {
					off: "No hotkeys are enabled.",
					when_blank: "Hotkeys are only enabled when there's nothing typed in the input window.",
					always_on: "Hotkeys are enabled at all times."
				}
			},
			hide_sidebar: {
				cat: "layout",
				type: ["options"],
				desc: "Whether to hide one or both of the sidebars.",
				def: "none",
				opt: {
					none: "Show both sidebars.",
					auto: "Automatically hide the left sidebar if the client is very narrow.  Not yet implemented.",
					left: "Hide the left sidebar.",
					right: "Hide the right sidebar.",
					both: "Hide both sidebars."
				},
				onChange: function(old) {
					var x = "";
					var y = "";
					if (prefs.hide_sidebar=="both") {
						x = "#left, #right {display:none;}";
						y = "#core {max-width: 100%;}";
					} else if (prefs.hide_sidebar=="left"||prefs.hide_sidebar=="right") {
						x = "#"+prefs.hide_sidebar+" {display:none;}";
						y = "#core {max-width: calc(100% - "+document.getElementById("right").offsetWidth+"px)}";
					}
					cssMods.hide_sidebar_bar = setStyle(x,cssMods.hide_sidebar_bar);
					cssMods.hide_sidebar_core = setStyle(y,cssMods.hide_sidebar_core);
				}
			}
		};
		//pref_options.sort(function(a, b) { 
		//	return a.cat > b.cat;
		//});
		function changePrefUI(pref) {
			//Todo: Track temporary changes for undo/save.
			setPref(pref, document.getElementById(pref).value, true);
		}
		function setPref(pref, val, ignore_same) {
			pref = pref.toLowerCase();
			if (typeof val == "string") {
				val = val.toLowerCase();
			}
			var info = pref_options[pref];
			if (!info) {
				return printUnscreened("There is no client preference named '" + safe_tags_replace(pref) +"'.", "client usererror");
			}
			var old = prefs[pref];
			if (old==val) {
				if (!ignore_same) {
					printUnscreened("The client preference '" + pref + "' is already set to '" + safe_tags_replace(val) + "'.", "client");
				}
				return;
			}
			switch (info.type[0]) {
				case "integer":
					val = parseInt(val, 10);
					if ((info.type.indexOf("nonnegative") > -1 && val < 0) || (info.type.indexOf("positive") > -1 && val < 1)) {
						return printUnscreened("The value " + safe_tags_replace(val) + " is too low for " + pref +".", "client usererror");
					}
					if ((info.type.indexOf("nonzero") > -1 && val == 0)) {
						return printUnscreened("The value for " + pref +" cannot be zero.", "client usererror");
					}
					break;
				case "options":
					if (!info.opt[val]) {
						return printUnscreened("The value " + safe_tags_replace(val) + " is not a valid option for " + pref +".", "client usererror");
					}
					break;
				case "string":
					if(info.type.indexOf("noquotes") > -1) {
						val = val.replace(/['"]+/g,'');
					}
					break;
				case "color":
					val = val.replace(/['"]+/g,'');
					break;
				default:
					return printUnscreened("Client error for preference " + pref + ".", "client error");
			}
			prefs[pref] = val;
			if (info.onChange) {
				info.onChange(old);
			}
			printUnscreened("Client Preference '" + pref + "' changed to '" + safe_tags_replace(val) + "'.", "client");
		}
		function openSettings() {
			showPrefs();
		}
		function saveSettings(clientOnly) {

		}
		function createSettingsInterface() {
			var preference_names = Object.keys(pref_options);
			var cats = [];
			var name, cat, ih, classes, current_value, valid_values, val, info;
			var cache = {};
			//addComponent(newID, parentID, newClass, clickFunction, clickArgs, newContents, title)
			addComponent("settings-ui", "body");
			addComponent("settings-tabtitles", "settings-ui");
			addComponent("settings-tabs", "settings-ui");
			addComponent("settings-footer", "settings-ui");
			addComponent("settings-savelocal", "settings-footer", false, saveSettings, [true], "<button>Client-Only Save</button>");
			addComponent("settings-save", "settings-footer", false, saveSettings, [false], "<button>Save Settings</button>");
			for (var i=0;i<preference_names.length;i++) {
				name = preference_names[i];
				info = pref_options[name];
				current_value = prefs[name];
				cat = info.cat;
				if (cats.indexOf(cat) === -1) {
					cats.push(cat);
					addComponent("settingstabtitle-"+cat, "settings-tabtitles", "tabtitle", showTab, [cat], cat);
					addComponent("settingstab-"+cat, "settings-tabs", "tabbody");
					cache[cat] = "<table><caption>" + cat + " Preferences</caption><th>Preference</th><th>Value</th><th>Description</th>";
				}
				ih = "<tr><td><label for=\"setting-"+name+"\">"+name+"</label><td>";
				switch (info.type[0]) {
					case "options":
						ih += "<select id=\"setting-"+name+"\">";
						valid_values = Object.keys(info.opt);
						for (var j=0;j<valid_values.length;j++) {
							val = valid_values[j];
							ih += "<option title=\"" + safe_tags_replace(info.opt[valid_values[j]]) + "\"" + (val===info.def?" class=\"client-defaultopt\"":"") + (val===current_value?" selected":"") + ">" + safe_tags_replace(val) + "</option>";
						}
						break;
					case "color":
						//Todo:  Abstract out the creation of span tags that identify current and default values, so the code's not being repeated so much.
						ih += "<input id=\"setting-"+name+"\" type=\"color\" defaultvalue=\""+info.def+"\" value=\""+current_value+"\">";
						break;
					case "integer":
					default:
						ih += "<input id=\"setting-"+name+"\" type=\"text`\" defaultvalue=\""+info.def+"\" value=\""+current_value+"\">";
				}
				ih += "</td><td>"+safe_tags_replace(info.desc)+"</td></tr>";
				cache[cat] += ih;
			}
			console.log(cats);
			for (cat in cats) {
				cat = cats[cat];
				console.log("About to fetch: settingstab-"+cat);
				console.log(document.getElementById("settingstab-"+cat));
				document.getElementById("settingstab-"+cat).innerHTML = cache[cat];
				console.log("Done with: settingstab-"+cat);
			}
			console.log("D");
		}
		function showTab(tab) {
			alert(tab);
			var t, title, contents;
			if (typeof(this) === "object" && this && this.className) {
				title = this;
				tab = this.id.substring(17);
			} else {
				title = document.getElementById("settingstabtitle-"+tab);
			}
			contents = document.getElementById("settingstab-"+tab);
			if (title.className.indexOf("activetabtitle") >= 0) return;
			for (t in document.getElementById("settings-tabtitles").children) {
				removeClass(t, "activetabtitle");
			}
			for (t in document.getElementById("settings-tabs").children) {
				removeClass(t, "activetab");
			}
			addClass(title, "activetabtitle");
			addClass(contents, "activetab");
		}
		function showPrefs() {
			//var table = ["Client Preferences:"];
			var table = ["<hr>"];
			var name;
			var current_value;
			var row;
			var info;
			var valid_values;
			var items;
			var classes;
			var preference_names = Object.keys(pref_options);
			var cat;
			var sortorder = {};
			for (var i=0;i<preference_names.length;i++){
				name = preference_names[i];
				console.log(name);
				cat = pref_options[name].cat;
				console.log(cat);
				items = sortorder[cat] || [];
				console.log(items);
				items.push(name);
				sortorder[cat] = items;
				console.log(sortorder);
			}
			console.log(sortorder);
			var preference_groups = Object.keys(sortorder);
			for (var q=0;q<preference_groups.length;q++) {
				cat = preference_groups[q];
				console.log(cat);
				preference_names = sortorder[cat];
				console.log(preference_names);
				table.push("<table>", "<caption>Client Preferences - " + cat.toUpperCase() + "</caption>", "<th>Preference</th><th>Settings</th><th>Description</th>");
				for (var i=0;i<preference_names.length;i++){
					name = preference_names[i];
					current_value = prefs[name];
					info = pref_options[name];
					if (info.dis) continue;
					row = "<tr><td class=\"client-optname\">" + safe_tags_replace(name) + "</td><td>";
					switch (info.type[0]) {
						case "options":
							valid_values = Object.keys(info.opt);
							items = [];
							for (var j=0;j<valid_values.length;j++) {
								classes = ((valid_values[j]==current_value?"client-currentopt":"") + (valid_values[j]==info.def?" client-defaultopt":"")).trim();
								items.push("<span title=\"" + safe_tags_replace(info.opt[valid_values[j]]) + "\"" + (classes?"class=\"" + classes + "\"":"") + ">" + safe_tags_replace(valid_values[j]) + "</span>");
							}
							row += items.join(" | ");
							break;
						case "color":
							//Todo:  Abstract out the creation of span tags that identify current and default values, so the code's not being repeated so much.
							if (current_value == info.def) {
								row += "<span class=\"client-currentopt client-defaultopt\">" + safe_tags_replace(String(current_value)) + "<span style=\"background-color:" + safe_tags_replace(String(current_value)) + "\">&nbsp;</span></span>";
							} else { 
								row += "<span class=\"client-currentopt\">" + safe_tags_replace(String(current_value)) + "<span style=\"background-color:" + safe_tags_replace(String(current_value)) + "\">&nbsp;</span></span> (<span class=\"client-defaultopt\">" + safe_tags_replace(String(info.def)) + "<span style=\"background-color:" + safe_tags_replace(String(info.def)) + "\">&nbsp;</span>)</span>)";
							}
							break;
						case "integer":
						default:
							if (current_value == info.def) {
								row += "<span class=\"client-currentopt client-defaultopt\">" + safe_tags_replace(String(current_value)) + (current_value==0&&info.on0?" ["+info.on0+"]":"") + "</span>";
							} else { 
								row += "<span class=\"client-currentopt\">" + safe_tags_replace(String(current_value)) + (current_value==0&&info.on0?" ["+info.on0+"]":"") + "</span>" + " (<span class=\"client-defaultopt\">" + safe_tags_replace(String(info.def))  + (info.def==0&&info.on0?" ["+info.on0+"]":"")+ "</span>)";
							}
					}
					row += "</td><td>" + info.desc + (info.on0?" [0 = "+info.on0+"]":"") + "</td></tr>"
					table.push(row);
				}
				table.push("</table>");
			}
			table.push("<div></div>");
			table.push("<div>Hover over an option for more details.  [<span class=\"client-currentopt\">current setting</span>] [<span class=\"client-defaultopt\">default setting</span>]</div>");
			table.push("<div>To change a setting, use: <strong>clientpref &lt;preferencename&gt; &lt;value&gt;</strong>.  To save settings for next time you open the client, use <strong>clientpref save</strong>.</div><hr>");
			table = table.join("");
			printScreened(table, "client-info");
		}
		var hotkey_mapping = {
			97: "go southwest",
			98: "go south",
			99: "go southeast",
			100:"go west",
			101:"look",
			102:"go east",
			103:"go northwest",
			104:"go north",
			105:"go northeast",
			106:"examine here",
			107:"go down",
			109:"go up",
			110:"exits",
			111:"inventory"
		}
		function savePrefs() {
			var toSave = {};
			for (var p in prefs) {
				if (prefs[p] != pref_options[p].def) {
					toSave[p] = prefs[p];
				}
			}
			toSave = JSON.stringify(toSave);
			if (localStorage && localStorage.setItem) {
				printUnscreened("Attempting to save preferences in local storage...", "client");
				localStorage.setItem("prefs", toSave);
				if (localStorage.getItem("prefs") != toSave) {
					printScreened("Saving preferences to local storage failed.", "client error");
				} else {
					printScreened("Preferences appear to have been successfully saved.", "client");
				}
			} else {
				printUnscreened("Local storage not found, not saving preferences in local storage.", "client");
			}
		}
		function saveMacros() {
			var toSave = JSON.stringify(macros);
			if(localStorage && localStorage.setItem) {
				printUnscreened("Attempting to save macros in local storage...", "client");
				localStorage.setItem("macros", toSave);
				if (localStorage.getItem("macros") != toSave) {
					printScreened("Saving macros to local storage failed.", "client error");
				} else {
					printScreened("Macros appear to have been successfully saved.", "client");
				}
			} else {
				printUnscreened("Local storage not found, not saving macros in local storage.", "client");
			}
		}
		// Based on http://code.is-here.com/zealotry/trunk/content/macro.js, function applyMacros
		function macroSubstitute(remainingInput) {
			var expansions = 0;
			var argsToDo = 0;

			var done = "";
			var arr;
			var argCount = 0;

			// fetch the next word
			while (arr = (/([a-zA-Z0-9_]+)/).exec(remainingInput)) {
				// any bits to the left of that word are added verbatim
				done += RegExp.leftContext;
				remainingInput = RegExp.rightContext;
				var word = arr[0];

				var macroMatch = macros[word];
				if (macroMatch) {
					// it's a macro; prepend it to the string we're working on
					var outStr = macroMatch.outStr;

					// then do macro-arg replacement (if any) on coming words
					argsToDo = macroMatch.args;
					argCount = 1;
					while (argsToDo > 0) {
						if (arr = (/([a-zA-Z0-9_]+)/).exec(remainingInput)) {
							// leftContext is discarded here as garbage
							remainingInput = RegExp.rightContext;
							word = arr[0];

							// make sure we're not inserting recursive junk
							if (word.indexOf("%") == -1) {
								// then substitute e.g. %1 for the new word
								while ((index = outStr.indexOf("%" + argCount)) != -1) {
									outStr = outStr.substring(0, index) + word +
										outStr.substring(index + 2);
								}
							}
						}
						argCount ++;
						argsToDo --;
					}
					// allow recursion: prepend 'input' rather than append 'done'
					remainingInput = outStr + remainingInput;

					expansions ++;
					if (expansions > 20) {
						printUnscreened("Too many macro expansions: aborting!", "client usererror");
						return null;
					}
				} else {
					// not a macro; accept the original word verbatim
					done += word;
				}
			}
			return done + remainingInput;
		}
		function initPrefs() {
			prefs = {};
			macros = {};
			for (var p in pref_options) {
				prefs[p] = pref_options[p].def;
			}
			if (localStorage && localStorage.getItem) {
				loadPrefString(localStorage.getItem("prefs"));
				loadMacroString(localStorage.getItem("macros"));
			}

			setActiveStyleSheet(prefs.theme);
		}
		function loadPrefString(p) {
			p = JSON.parse(p);
			for (var pref in p) {
				setPref(pref, p[pref], true);
			}
		}
		function loadMacroString(m) {
			m = JSON.parse(m);
			macros = {};
			for (var macro in m) {
				macros[macro] = m[macro];
			}
		}
		// Based on http://code.is-here.com/zealotry/trunk/content/macro.js
		function addMacro(inStr, outStr) {
			console.log("Setting macro " + inStr + " to " + outStr);

			var newMacro = new Object();
			newMacro = new Object();
			newMacro.inStr = inStr;
			newMacro.outStr = outStr;
			newMacro.args = 0;

			var index;

			while ((index = outStr.indexOf("%", index)) != -1) {
				index++;
				if (outStr[index] >= '1' && outStr[index] <= '9') {
					var argIndex = outStr[index] - '0';
					if (argIndex > newMacro.args) {
						newMacro.args = argIndex;
					}
				}
			}
			macros[inStr] = newMacro;
		}

		function init() {
			removeEvent(window, 'load', init, {once:true});
			output = document.getElementById("output");
			input  = document.getElementById("commandinput");
			addEvent(input, "keydown", keyDown);
			addEvent(input, "mouseup", keyUp, {passive:true});
			addEvent(input, "keyup", keyUp, {passive:true});
			addEvent(input, "focus", restorePos, {passive:true});
			addEvent(window, "mouseup", fixFocus, {passive:true});
			addEvent(window, "resize", keepScrollPos, {passive:true});
			addEvent(output, "mouseup", checkOutputClick);
			addEvent(output, "scroll", noteScrollPos, {passive:true});
			addEvent(output, "paste", redirectPaste, {passive:true});
			initPrefs();
			initTheatre();
			recalcInputLineHeight();
			debugMsg("Client initialized and ready for connection.");
			initConnection();
			window.focus();
			c.raw = window.location == "http://test.skotos.net/orchil/marrach/marrach.htm";
		}

	//-----General UI
		function addComponent(newID, parentID, newClass, clickFunction, clickArgs, newContents, newTitle) {
			var parent = (typeof parentID == "object" ? parentID : document.getElementById(parentID));
			if (!parent) reportClientError("Cannot find parent " + parentID);
			var newComponent = document.createElement('div');
			parent.appendChild(newComponent);
			if (newID) newComponent.id = newID;
			if (newClass) newComponent.className = newClass;
			if (newContents) newComponent.innerHTML = newContents;
			if (newTitle) newComponent.title = newTitle;
			if (clickFunction) {
				var fun;
				if (typeof(clickFunction) == "string") {
					var def = clickFunction+'("'+clickArgs.join('","')+'")';
					fun = new Function(def);
					//writeToConsole("Writing clickFunction " + def);
				} else {
					fun = clickFunction;
					//writeToConsole("Writing clickFunction " + fun);
					newComponent.dataset.clickargs = clickArgs;
				}
				addEvent(newComponent, "click", fun, {passive:true});
				newComponent.style.cursor = "pointer";
			}
			return newComponent;
		}
		function connConnected() {
			conn.connected = true;
			printScreened("CONNECTED", "connection");
			input.contentEditable = true;
			input.className = input.className.replace(/\bdisabled\b/,'');
			input.placeholder = "Enter a command...";

			//input.removeAttribute("disabled");
			input.disabled = false;
			setInputValue("");
			fixFocus();
			if (!(c.raw)) sendSys("SKOTOS Orchil 0.2.3");
		}
		function connDisconnected() {
			conn.connected = false;
			input.contentEditable = false;
			input.className += ' disabled';
			input.disabled = true;
			printScreened("DISCONNECTED", "connection");
			input.placeholder = "";
			conn = null;
			if (prefs.prompt_savelog=="on" && loginDone) {
				saveCurrentWindow();
			}
		}
		function fixFocusIfUnselected(e) {
			if (prefs.output_click == "focus_output" ||
			   (typeof window.getSelection != "undefined" && window.getSelection().toString()) ||
			   (typeof document.selection != "undefined" && document.selection.type == "Text" && document.selection.createRange().text)) {
				cancelEvent(e);
			}
		}
		function checkOutputClick(e) {
			if (isLeftClick(e)) {
				//Todo: Test/debug this.
				//cancelEvent(e);
				fixFocusIfUnselected(e);
			} else {
				cancelEvent(e);
			}
		}
		function isLeftClick(e) {
			var b;
			e = e || window.event;
			if ("which" in e)  //Gecko/WebKit/Opera/New IE
				return e.which < 2;
			else if ("button" in e)  //Old IE/Opera
				return e.button < 2;
			return true; //We don't know, so assume yes.
		}
		function isConnected() {
			return input.className.indexOf("disabled") == -1;
		}
		var currentlyAdjustingScroll;
		var currentAdjustingMode;
		function updateAutomaticScroll() {
			if (currentlyAdjustingScroll || !(prefs.horizontal_scroll in ["automatic","full_adaptive"])) return;
			currentlyAdjustingScroll = true;
			setTimeout(adjustingScrollTimeout, 100);
		}
		function adjustingScrollTimeout() {
			if(!(prefs.horizontal_scroll in ["automatic","full_adaptive"])) {
				currentlyAdjustingScroll = false;
				return;
			}
			var flow = "hidden";
			if (scrolledToBottom || prefs.horizontal_scroll == "full_adaptive") {
				var bonus = 20; //Todo: Figure out a better value for this.
				var heightLeftToCheck = output.scrollHeight + bonus;
				var node;
				for (var i=output.childNodes.length - 1; i>=0; i--) {
					node = output.childNodes[i];
					if (node.clientWidth != node.scrollWidth) {
						flow = "auto";
						break;
					}
					heightLeftToCheck -= node.scrollHeight;
					if (heightLeftToCheck < 0) {
						flow = "hidden";
						break;
					}
				}
			} else {
				flow = "auto";
			}
			if (flow != currentAdjustingMode) {
				cssMods.pre_wrap = setStyle("#output.pre {white-space:pre;} #output {overflow-x:"+flow+";}",cssMods.pre_wrap);
				currentAdjustingMode = flow;
			}
			currentlyAdjustingScroll = false;
		}
		function updateAdaptiveScroll() {
			//Todo: actual adaptive scroll.  Until then, just do automatic.
			//Adaptive loops through all 'visible' elements always, instead of "all bottom elements if at bottom".
			//Note that adaptive can be intensive in times of heavy scroll.  Perhaps install a limiter.
			//Like start with if (adaptive_scrolling) return; adaptive_scrolling = true;
			//Then at the end do a setTimeout to set adaptive_scrolling = false;
			updateAutomaticScroll();
		}
	//-----Input
		function fixFocus(e) {
			e = e || window.event || {};
			if (isConnected() && !e.noFixFocus) {
				input.focus();
			}
		}
		function doSend(message, noecho) {
			if (!conn.connected) return;
			var debug = false;
			closeAllSubElements();
			if (prefs.local_echo === "off") noecho = true;
			if(!noecho) {
				if (c.promptStr) {
					printUnscreened(message, "inputecho");
					//test
				}
				printUnscreened(message, "inputecho");
			} else if (debug) {
				debugMsg("UI Sending: " + safe_tags_replace(message));
			}
			scrollToBottom();
			if (true) {
				if (c.raw) writeToConsole("S:"+message+"\n");
				conn.send(message+"\n");
			} else {
				if (message==="\n") {
					console.log("It's just a newline!");
					message = [""];
				} else {
					message = message.replace("\r").split("\n");
				}
				for (var i=0;i<message.len;i++) {
					writeToConsole("S:"+message[i]+"\n");
					conn.send(message[i]+"\n");
				}
			}
			fixFocus();
		}
		function sendCommand() {
			var command = replaceBadMSCharacters(getInputValue());
			var lc = command.toLowerCase();
			if (lc==="clientpref"||lc==="clientprefs"||lc==="@clientpref"||lc==="@clientprefs") {
				showPrefs();
			} else if (lc.substring(0,11)==='clientpref '||lc.substring(0,12)==='clientprefs '
				||lc.substring(0,12)==='@clientpref '||lc.substring(0,13)==='@clientprefs ') {
				var components = command.split(" ");
				if (components.length < 3) {
					if (components[1] === "save") {
						savePrefs();
					} else {
						printUnscreened("Usage: clientpref <preferencename> <value> or clientpref save", "client usererror");
					}
				} else {
					setPref(components[1], components.slice(2, 100).join(" "));
				}
			} else if (lc==="macro" || lc === "@macro" || lc==="macros" || lc==="@macros") {
				printUnscreened("Your macros:", "client");
				for (var m in macros) {
					printUnscreened("  " + m + ": " + macros[m].outStr, "client");
				}
				printUnscreened("============", "client");
				printUnscreened("To add a macro type: macro add <macroname> <macro text>..., e.g. macro add key1 my belt's pouch's keyring's key", "client");
				printUnscreened("To delete a macro type: macro clear <macroname>", "client");
			} else if (lc.substring(0,6) === 'macro ' || lc.substring(0,7) === "@macro "
				|| lc.substring(0,7) === 'macros ' || lc.substring(0,8) === "@macros ") {
				var components = command.split(" ");
				if(components[1] === "add" && components.length >= 4) {
					addMacro(components[2], components.slice(3).join(" "));
					saveMacros();
				} else if (components[1] === "clear" && components.length == 3) {
					console.log("Deleting macro " + components[2]);
					delete macros[components[2]];
					saveMacros();
				} else {
					printUnscreened("Usage: macro add <macroname> macro_values... or macro clear <macroname>", "client usererror");
				}
			} else {
				doSend(macroSubstitute(command));
			}
			saveHistory(command);
			if (prefs.keep_last_command !== "on") {
				historyPosition = 0;
				//savedInput = ""; -- did this do anything?  Looks like null code.
				setInputValue("");
				resizeInput(true);
			} else {
				historyPosition = 1;
				input.select();
			}
		}
		function saveHistory(line) {
			if (line.length >= prefs.command_history_min && commandHistory[commandHistory.length-1] != line) {
				commandHistory.push(line);
				if (prefs.command_history_lines && commandHistory.length > prefs.command_history_lines) {
					commandHistory.shift();
				}
			}
		}
		function sendTitle() {
			if (this && typeof this == 'object') {
				sendUI(this.title);
			} else {
				reportClientError("Clickies not supported.");
				printScreened("<span style='color: red;'>ERROR:</span> Browser does not support modern event references; clicking some elements will not work.", "client error");
			}
		}
		function sendDataCommand() {
			if (this && typeof this == 'object') {
				sendUI(this["data-command"]);
			} else {
				reportClientError("Clickies not supported.");
				printScreened("<span style='color: red;'>ERROR:</span> Browser does not support modern event references; clicking some elements will not work.", "client error");
			}
		}
		function sendUI(message) {
			//debugMsg("UI SEND: " + message, "sending");
			doSend(message, 1);
		}
		function sendSys(message) {
			var debugMode = false;
			doSend(message, !debugMode);
		}
		function keyDown(e) {
			if (!isConnected()) {
				e.preventDefault();
				return false;
			}
			var k = (window.event?window.event.keyCode:(e?e.which:null));
			if (k===null) return true;
			switch (k) {
				case 13: //Enter by itself sends commands.
					if (noModifier(e)) {
						sendCommand();
						e.preventDefault();
						return false;
					}
					break;
				case 9: //Tabbing to other elements is not useful.
					e.preventDefault();
					return false;
				case 33: //Page Up to scroll up the output screen.
					if (noModifier(e)) {
						outputPageUp();
						e.preventDefault();
					}
					break;
				case 34: //Page Down to scroll down the output screen.
					if (noModifier(e)) {
						outputPageDown();
						e.preventDefault();
					}
					break;
				case 38: //Up Arrow to get the previous command.
					//May not detect "at selection start" for IE.
					//Oh well, not important, and only matters if they change prefs anyway.
					if (e.shiftKey || inputCursorLine() > 1)
						break;
					if (prefs.arrow_behavior == "current_first") {
						if (e.ctrlKey || input.selectionStart==0) {
							commandHistoryPrevious();
							e.preventDefault();
						} else {
							input.selectionStart = 0;
						}
					} else if (noModifier(e)) {
						commandHistoryPrevious();
						e.preventDefault();
					}
					break;
				case 40: //Down arrow to get the next command.
					//May not detect "at selection start" for IE.
					//Oh well, not important, and only matters if they change prefs anyway.
					if (e.shiftKey || inputCursorLine() < inputLines())
						break;
					if (prefs.arrow_behavior == "current_first") {
						if (e.ctrlKey || input.selectionStart==input.value.length) {
							commandHistoryNext();
							e.preventDefault();
						} else {
							input.selectionStart = input.value.length;
						}
					} else if (noModifier(e)) {
						commandHistoryNext();
						e.preventDefault();
					}
					break;
				default:
					if (hotkey_mapping[k] && (prefs.hotkeys_enabled == "always_on" || (prefs.hotkeys_enabled == "when_blank" && !getInputValue()))) {
						doSend(hotkey_mapping[k], true);
						e.preventDefault();
					}
			}
			return true;
		}
		function noModifier(e) {
			return !(e.ctrlKey) && !(e.shiftKey) && !(e.altKey);
		}
		HTMLTextAreaElement.prototype.insertAtCaret = function (text) {
			text = text || '';
			if (document.selection) {
				// IE
				this.focus();
				var sel = document.selection.createRange();
				sel.text = text;
			} else if (this.selectionStart || this.selectionStart === 0) {
				// Others
				var startPos = this.selectionStart;
				var endPos = this.selectionEnd;
				this.value = this.value.substring(0, startPos)+text+this.value.substring(endPos, this.value.length);
				this.selectionStart = startPos + text.length;
				this.selectionEnd = startPos + text.length;
			} else {
				this.value += text;
			}
		};
		function redirectPaste(e) {
			var pastedText = undefined;
			if (window.clipboardData && window.clipboardData.getData) { // IE
					pastedText = window.clipboardData.getData('Text');
			} else {
				var clipboardData = (e.originalEvent || e).clipboardData;
				if (clipboardData && clipboardData.getData)
					pastedText = clipboardData.getData('text/plain');
			}
			if (pastedText) {
				input.insertAtCaret(pastedText);
				input.focus();
			}
			return false;
		}
		var scrolledToBottom = true;
		var inputPos;
		var commandHistory = [];
		var historyPosition = 0;
		var currentInput = "";
		function commandHistoryPrevious() {
			if (historyPosition < commandHistory.length) {
				if (historyPosition == 0) currentInput = getInputValue();
				historyPosition++;
				setInputValue(commandHistory[commandHistory.length - historyPosition]);
			}
		}
		function commandHistoryNext() {
			if (historyPosition) {
				historyPosition--;
				setInputValue(historyPosition ? commandHistory[commandHistory.length - historyPosition] : currentInput);
			}
		}
		function getInputValue() {
			if (input.tagName==="TEXTAREA") {
				input.placeholder = "";
				return input.value;
			} else {
				if (document.body.innerText) {
					writeToConsole("Using innerText");
					 return input.innerText;
				} else {
					writeToConsole("Using innerHTML");
					 return input.innerHTML.replace(/\&lt;br\&gt;/gi,"\n").replace(/(&lt;([^&gt;]+)&gt;)/gi, "");
				}
			}
		}
		function setInputValue(text) {
			if (input.tagName==="TEXTAREA") {
				input.value = text;
			} else {
				if (document.body.innerText) {
					writeToConsole("Setting innerText");
					 input.innerText = text;
				} else {
					writeToConsole("Setting innerHTML");
					 input.innerHTML = text.replace(/\n/gi,"\&lt;br\&gt;"); //Todo: Test this.
				}
			}
		}
		function keyUp(e) {
			savePos();
			resizeInput();
		}
		function recalcInputLineHeight(force) {
			var h;
			//Try the easy way first.
			if (input.currentStyle) {
				h = input.currentStyle["line-height"];
				if (h.slice(-2)=="px") {
					h = Number(h.slice(0,-2));
					if (!(h>0)) h = false;
				} else {
					h = false;
				}
			}
			if (!h) {
				//Try the 'hard' way otherwise.
				var tmp = getInputValue();
				setInputValue("\n\n\n\n\n\n\n\n\n");
				h = input.scrollHeight / 10;
				setInputValue(tmp);
			}
			if (force || !(c.inputLineHeight) || h!=c.inputLineHeight) {
				c.inputLineHeight = h;
				input.style.minHeight = (h * prefs.input_minheight) + "px";
				resizeInput();
			}
		}
		function resizeInput(reset) {
			noteScrollPos();
			var lheight = c.inputLineHeight;
			input.style.height = "2.3em";
			input.style.height = Math.min(input.scrollHeight + 4, 105) + "px";
			keepScrollPos();
		}
		function savePos() {
			if (input.tagName!=="TEXTAREA") {
				inputPos = window.getSelection ? window.getSelection().getRangeAt(0) : (document.selection?document.selection.createRange():null);
			}
		}
		function restorePos() {
			if (input.tagName!=="TEXTAREA") {
				if (inputPos !== null && input.innerText) {
					if (window.getSelection) { //non IE and there is already a selection
						var s = window.getSelection();
						if (s.rangeCount > 0) 
							s.removeAllRanges();
							s.addRange(inputPos);
					} else if (document.createRange) { //non IE and no selection
						window.getSelection().addRange(inputPos);
					} else if (document.selection) { //IE
						inputPos.select();
					}
				}
			}
		}
		function inputCursorLine() {
			//TODO: Make this work properly with wordwrap, instead of only detecting newline characters.
			return input.value.substr(0, input.selectionStart).split("\n").length;
		}
		function inputLines() {
			//TODO: Match any updates to inputCursorLine
			return input.value.split("\n").length;
		}
	//-----Output
		function printScreened(message, cls) {
			if (!output) console.log("No output for: "+message);
			//Prints a line element independent of any current streamed element.
			noteScrollPos();
			var ele = newLineElement("div", cls);
			ele.innerHTML = message;
			if (prefs.scrollback_count && output.childNodes.length > prefs.scrollback_count) {
				output.removeChild(output.firstChild);
			} else if (prefs.scrollback_count && output.childNodes.length == prefs.scrollback_count) {
				writeToConsole("Scrollback has reached " + prefs.scrollback_count + " elements; older lines will now be removed to limit client lag.");
			}
			keepScrollPos();
			if (prefs.horizontal_scroll==="automatic") {
				updateAutomaticScroll();
			}
		}
		function noteScrollPos() {
			if (!receiveActive) scrolledToBottom = (output.scrollTop >= (output.scrollHeight - output.offsetHeight));
		}
		function keepScrollPos() {
			if (!receiveActive && scrolledToBottom) scrollToBottom();
		}
		function scrollToBottom() {
			output.scrollTop = output.scrollHeight;
			scrolledToBottom = true;
		}
		function printUnscreened(message, cls) {
			printScreened(safe_tags_replace(message), cls);
		}
		function writeToConsole(message) {
			console.log(message);
			//printScreened("<span style='color: orange;'>CONSOLE:</span> " + safe_tags_replace(message));
		}
		function plog(arg1, arg2, arg3, arg4, arg5, arg6) {
			//console.log(arg1, arg2, arg3, arg4, arg5, arg6);
		}
		function outputPageUp() {
			if (output.scrollBy) {
				output.scrollBy(0,0-output.clientHeight);
			} else {
				output.scrollTop = output.scrollTop - output.clientHeight;
			}
			fixFocus();
		}
		function outputPageDown() {
			if (output.scrollBy) {
				output.scrollBy(0,output.clientHeight);
			} else {
				output.scrollTop = output.scrollTop + output.clientHeight;
			}
			fixFocus();
		}
		var copyToClipboardFailed = false;
		function copyOutputSelection() {
			if (copyToClipboardFailed ||
			   (typeof window.getSelection != "undefined" && !(window.getSelection().toString())) ||
			   (typeof document.selection != "undefined" && (document.selection.type != "Text" || !(document.selection.createRange().text)))
			) {
				return;
			}
			var success = false;
			try {
				success = document.execCommand("copy");
			} catch(e) {
				success = false;
			}
			if (!success) {
				copyToClipboardFailed = true;
				printScreened("<span style='color: red;'>ERROR:</span> Copy-on-select unsuccessful.  Make sure to use an up-to-date browser.  This functionality has been disabled for this session; to turn it off permanently,  enter <strong>clientpref highlight_to_copy off'.", "client error");
			}
		}
//-----Interpreter Code
	var receiveExtra = '';
	var receiveMode = 'RAW';
	var currentSubElements = [];
	var receiveActive = false;
	var loginDone = false;
	function doReceive(msg) {
		//writeToConsole("ARG");

		if (c.raw) writeToConsole("R:"+msg);

		//debugMsg("Receive: " + msg);
		noteScrollPos();
		receiveActive = true;
		msg = receiveExtra + msg;
		var lines = msg.split('\r\n');
		receiveExtra = lines.pop();

		for (var i=0; i<lines.length; i++) {
			var str = lines[i];

			if(str.length > 0 && !document.hasFocus()) {
				if(!document.title.includes("*")) {
					document.title = "* " + document.title;
				}
			}
			if (str.substring(0,7)==='SECRET ') {
				var userName = loadCookie("user");
				var passWord = loadCookie("pass");
				var secret = str.substring(7, str.length);
				var hash = hexMD5(userName + passWord + secret);

				// Skotoslib uses a different authentication method. Account for that here.
				if(generic == true) {
					//passWord = hexMD5(passWord);
					hash = hexMD5(userName + passWord + secret);
				}

				sendSys("USER " + userName);
				sendSys("SECRET " + secret);
				sendSys("HASH " + hash);

				if (window.charName && window.charName.length > 0) {
					sendSys("CHAR " + window.charName);
				}

				if(hasChars == true) {
					sendSys("CHAR " + getChar());
				} else {
					gameCharacter = userName;
				}

				receiveMode = 'ALICECOMPAT';
				loginDone = true;
			} else if (str.substring(0,6)==='SKOOT ') {
				var sppos = str.indexOf(" ", 6);
				if (sppos == -1) {
						badSkoot('?', str);/* malformed SKOOT */
				} else {
					var seq = str.substring(6, sppos);
					doSkoot(seq, str.substring(sppos + 1));
				}
			} else if (str.substring(0,7)==='MAPURL ') {
				var url = str.substring(7);
				var img = document.getElementById("image_map_img");
				if (sppos == -1 || !img) {
					badSkoot('MAPURL', str);
				} else {
					console.log(str);
					img.style.backgroundImage = "url('"+url+"')";;
				}
			} else {
				if (str==="An error occurred while processing your login.") {
					loginDone = false;
					openerWin("https://www.skotos.net/user/login.php?redirect=http%3a%2f%2ftest%2eskotos%2enet%2forchil%2f");
				}
				processInput(str);
			}
		}
		if (receiveExtra && receiveExtra.indexOf('SKOOT') != 0 && receiveExtra.indexOf('SECRET')) {
			msg = receiveExtra;
			receiveExtra = '';
			processInput(msg, true);
		}
		receiveActive = false;
		keepScrollPos();
	}
	function processInput(line, nonl /*=false*/) {
		switch (receiveMode) {
			case 'RAW': //Dangerous & Stupid
				printScreened(line);
				break;
			case 'PLAIN': //Safe, but low on features
				printUnscreened(line, "pre");
				break;
			case 'ANSI': //The old standby
				//Figure this out later, if ever.
				printUnscreened(line, "pre");
				break;
			case 'ALICECOMPAT': //What the servers support by default.
				parseAliceCompat(line, nonl);
				break;
			case 'PRE': //Temporarily in a PRE tag; this seems unused.
				parsePreTag(line, nonl);
				break;
			default:
				alert("Bad receive mode.");
		}
	}
	function parseAliceCompat(str, nonl) {
		var tagdef;
		if (!str.length && !nonl) {
			newLineElement();
			//addToCurrentElement("\n");
		}
		while (str.length) {
			var pos = str.indexOf("<");
			if (pos === -1) {
				addToCurrentElement(str);
				break;
			} else {
				if (pos !==0) {
					addToCurrentElement(str.substring(0,pos));
				}
				str = str.substring(pos+1);
				//At this point, we are at the start of an open or close tag.
				//Now we split on > to get everying inside - name, and possibly / or attributes.
				pos = str.indexOf(">");
				if (pos === -1) {
					receiveExtra += "<" + str; //The tag opening isn't complete; save what remains in case we get the rest later.
					break;
				}
				tagdef = str.substring(0,pos).trim();
				str = str.substring(pos+1);
				if (tagdef.charAt(0) === "/") {
					closeElement(tagdef.substring(1));
				} else {
					pos = tagdef.indexOf(" ");
					if (pos === -1) {
						openElement(tagdef);
					} else {
						openElement(tagdef.substring(0, pos), tagdef.substring(pos+1));
					}
				}
			}
		}
		if (!nonl) {
			plog("Closing non-multi because !nonl");
			closeNonMultiElements();
		}
		if (prefs.horizontal_scroll==="automatic") {
			updateAutomaticScroll();
		}
	}
	function closeNonMultiElements() {
		plog("Closing non-multi elements", currentSubElements);
		var pos = currentSubElements.length - 1;
		var tag;
		while (pos>-1) {
			if (["pre","ul","ol"].indexOf(currentSubElements[pos].tagName.toLowerCase()) !== -1) {
				break;
			}
			currentSubElements.splice(-1);
			plog("Removed one subelement.", currentSubElements);
			pos--;
		}
	}
	function closeAllSubElements() {
		currentSubElements.splice(0);
		plog("Removed all subelements.", currentSubElements);
	}

	function addToCurrentElement(text) {
		if (!text || text=="") {
			return;
		}

		text = remove_non_ascii(text);

		var cur = getCurrentSubElement();
		noteScrollPos();
		plog("Writing: ",text,"To: ", cur);
		cur.insertAdjacentHTML('beforeend', text);
		keepScrollPos();
	}

	function remove_non_ascii(str) {
  		if ((str===null) || (str===''))
       		return false;
 		else
   			str = str.toString();
  
  		return str.replace(/[^\x20-\x7E]/g, '');
	}

	function unEscape(text) {
		return text.replace(/&quot;/g, '"').replace(/&gt;/g, '>').replace(/&lt;/g, '<').replace(/&amp;/g, '&');
	}
	function getCurrentSubElement() {
		if (!currentSubElements.length || (currentSubElements.length===1 && currentSubElements[0].tagName==="PRE")) {
			plog("Creating new div as current subelement.");
			currentSubElements.push(newLineElement());
		}
		return currentSubElements[currentSubElements.length-1];
	}
	var tagConversions = {
		"font":"span",
		"span":"span",
		"hr":"hr",
		"b":"strong",
		"strong":"strong",
		"i":"em",
		"em":"em",
		"pre":"pre",
		"ol":"ol",
		"ul":"ul",
		"li":"li",
		"center":"div",
		"a":"a",
		"table":"table",
		"caption":"caption",
		"tbody":"tbody",
		"tr":"tr",
		"th":"th",
		"td":"td"
	};
	function openElement(tag, attributes) {
		var ele;
		if (tag==="body") {
			// Body tag can have attributes including bgcolor, text, link, plink, vlink, topmargin, leftmargin, marginwidth, marginheight
			// Example: Received body tag bgcolor='#000000' text='#FF6103' link='#cd0000' plink='#cd0000'
			return applySkotosTheme(attributes);
		}
		if (tag==="xch_page") {
			return applyXchPage(attributes);
		}
		var nutag = tagConversions[tag];
		if (!nutag) {
			return writeToConsole("Invalid tag: " + tag);
		}
		if (nutag==="li") {
			closeElement("li", true);
		}
		noteScrollPos();
		plog("opening: " + tag + "/" + nutag);
		if (["hr","div","p","table","pre"].indexOf(nutag) !== -1) {
			closeNonMultiElements();
			ele = newLineElement(nutag);
		} else {
			ele = document.createElement(nutag);
			plog("Appended new element:", ele, "To:", getCurrentSubElement());
			getCurrentSubElement().appendChild(ele);
		}
		if(tag==="font" && attributes==="size=+1") {
			debugtrack = ele;
		}
		currentSubElements.push(ele);
		var attr = parseAttributes(attributes);
		if (tag==="center") {
			attr["class"] = "centertag";
		}
		if (attr["class"]) {
			ele.className = attr["class"];
		}
		if (attr["style"]) {
			ele.style.cssText = attr["style"];
		}
		if (attr["xch_cmd"]) {
			ele.title=attr["xch_cmd"];
			addEvent(ele, "click", sendTitle, {passive:true});
		} else if (attr["title"]) {
			//Optional TODO: Support titles and xch_cmds being different
			ele.title=attr["title"];
		}
		if (tag==="hr") {
			closeElement("hr");  //<hr> doesn't get inner content.
		}
		keepScrollPos();
	}
	function applySkotosTheme(attributes) {
		var outputAttributes = "";
		var extraEntries = "";

		console.log("applySkotosTheme", attributes);
		console.log("Existing theme choice:", prefs.theme);

		// Save these in case we enable the SkotOS theme later
		skotosAttributes = attributes;

		// Only apply a SkotOS theme if that's what the user wants.
		// Don't fight with other enabled CSS.
		if (prefs.theme !== "skotos") {
			console.log("Clear SkotOS styling...");
			cssMods.skotosAttributes = setStyle("", cssMods.skotosAttributes);
			return;
		}

		let attrs = attributes.split(" ");
		for(var i = 0; i < attrs.length; i++) {
			if(attrs[i].substring(0, 8) == "bgcolor=") {
				let bgColor = stripQuotes(attrs[i].substring(8));
				outputAttributes += "background-color:" + bgColor + ";";
			} else if (attrs[i].substring(0,5) == "text=") {
				let textColor = stripQuotes(attrs[i].substring(5));
				outputAttributes += "color:" + textColor + ";";
			} else if (attrs[i].substring(0,5) == "link=") {
				let linkColor = stripQuotes(attrs[i].substring(5));
				extraEntries += "\n#output a {color:" + linkColor + " !important}"
			} else if (attrs[i].substring(0,6) == "vlink=") {
				let linkColor = stripQuotes(attrs[i].substring(6));
				extraEntries += "\n#output a:visited {color:" + linkColor + " !important} "
			} else if (attrs[i].substring(0,6) == "alink=") {
				let linkColor = stripQuotes(attrs[i].substring(6));
				extraEntries += "\n#output a:active {color:" + linkColor + " !important}"
			}
			// Various unhandled attributes exist and we're ignoring them.
		}

		let cssText = "#output {" + outputAttributes + "} #commandinput {" + outputAttributes + "}" + extraEntries;
		console.log("Skotos styling:", cssText);
		// Set the SkotOS-theme CSS
		cssMods.skotosAttributes = setStyle(cssText, cssMods.skotosAttributes);
	}
	function applyXchPage(attributes) {
		if (attributes==="clear=\"text\"" || attributes==="clear=\"text\" /") {
			while (output.firstChild) {
				output.removeChild(output.firstChild);
			}
		} else {
			writeToConsole("Unexpected attributes for xch_page: " + attributes);
		}
	}
	function closeElement(tag, lastOnly /*=false*/) {
		var nutag = tagConversions[tag];
		plog("closing: " + tag + "/" + nutag + "; " + lastOnly);
		plog(currentSubElements);
		var l = currentSubElements.length;
		if (l) {
			if (lastOnly) {
				if (currentSubElements[l-1].tagName.toLowerCase()===nutag) {
					currentSubElements.splice(-1);
				}
			} else {
				for (var i=l-1; i>=0; i--) {
					if (currentSubElements[i].tagName.toLowerCase()===nutag) {
						currentSubElements.splice(i, l-i);
						break;
					}
				}
			}
		}
		plog(currentSubElements);
		plog("done closing");
	}
	var gencolors = {
		"#646464":"grey",
		"#ffffff":"white",
		"#e12000":"red",
		"#00cd20":"green",
		"#ff9b00":"orange",
		"#0020ff":"blue",
		"#e119e1":"pink",
		"#00cde1":"cyan",
		"#000000":"black",
		"#FF00":"black",
		"#9B30":"black",
        "#CC33":"green",
        "#FF33":"pink",
        "#FFCC":"orange",
        "#07B3":"blue",
        "#BED3":"cyan"
	};
	function parseAttributes(attr) {
		if (!attr) {
			return {};
		}

		if (attr==="size=+1") {
			return {"class":"genbig"};
		}
		if (attr.substring(0,6)==="color=") {

			attr = stripQuotes(attr.substring(6));
			if (gencolors[attr]) {
				return {"class":"gen"+gencolors[attr]};
			}
			return {"style":"color:"+attr};
		}
		if (attr.substring(0,6)==="class=") {
			attr = stripQuotes(attr.substring(6));
			return {"class":attr};
		}
		if (attr.substring(0,8)==="xch_cmd=") {
			attr = stripQuotes(attr.substring(8));
			return {"xch_cmd":attr};
		}
		if (attr.substring(0,6)==="title=") {
			attr = stripQuotes(attr.substring(6));
			return {"title":attr};
		}
		if (attr) {
			reportClientError("Unknown attr: " + attr );
		}
		return {};
	}
	function stripQuotes(txt) {
		var e = txt.length;
		var s = (txt.charAt(0)==='"' || txt.charAt(0)==="'") ? 1 : 0;
		e = (txt.charAt(e-1)==='"' || txt.charAt(e-1)==="'") ? e - 1 : e;
		return txt.substring(s, e);
	}
	function newLineElement(type /*="div"*/, cls) {
		noteScrollPos();
		if (!type) type="div";
		var ele = document.createElement(type);
		if (cls) {
			ele.className = cls;
		}
		if (currentSubElements.length && currentSubElements[0].tagName==="PRE") {
			//addToCurrentElement("\n");
			currentSubElements[0].appendChild(ele);
		} else {
			output.appendChild(ele);
		}
		keepScrollPos();
		return ele;
	}
	function badSkoot(num, msg) {
		reportClientError('Bad SKOOT message received: ' + num + " " + msg);
	}
	function reportClientError(msg) {
		//sendSys('@CLIENTERR ' + msg);
		console.log('CLIENTERRORREPORT ' + msg);
	}

	function getChar() {
		var charName = getUrlParam("charName", undefined);

		if(charName) {
			return charName;
		}

		return prompt("Character");
	}

	function loadCookie(name) {

		var allcookies = document.cookie;
		if (!allcookies) {
			if (name=="user") {
					return prompt("Username");
			}
			if (name=="pass") {
					if(generic == true) {
						return hexMD5(prompt("Password"));
					} else {
						return prompt("Password");
					}
			} 
		}
		var start = allcookies.indexOf(name + "=");
		if (start == -1) {
			if (name=="user") {
					return prompt("Username");
			}
			if (name=="pass") {
					return prompt("Password");
			} 
			return null;
		}
		start += name.length + 1;
		var end = allcookies.indexOf(';',start);
		if (end == -1) {
			end = allcookies.length;
		}

		return allcookies.substring(start,end);
	}
//-----Window Code
	window.openerLoc = "";
	window.openerName = "";
	if (window.opener && !window.opener.closed) {
		try {
			//This may error out if the opener navigated away due to CORS.
			window.openerLoc  = window.opener.location.href;
			window.openerName = window.opener.window.name;
		} catch(e) {
			console.log(e);
		}
	}
	document.popups = {};
	function openerWin(filename) {
		if (filename != "") {
			if (!window.opener || window.opener.closed) {
				window.open(filename, window.openerName);
				return;
			} else {
				window.opener.focus();
				window.opener.location.href = filename;
				return;
			}
		} else {
			if (!window.opener || window.opener.closed) {
				window.open(window.openerLoc, window.openerName);
				return;
			} else {
				window.opener.focus();
				return;
			}
		}
	}
	function popupArtWin(filename, windowname, windowtitle) {
		var scrLeft = 16 + window.screenLeft;
		var scrTop  = 16 + window.screenTop;
        
		document.popups[windowname] = {};
		document.popups[windowname].src = filename;
		document.popups[windowname].title = windowtitle;
		var artwin = open('../art.htm', windowname, 'width=128,height=128,left=' + scrLeft + ',top=' + scrTop + 'hotkeys=no,scrollbars=no,resizable=no');
		popupFollowUp(filename, artwin);
	}
	function popupWin(filename, windowname, remWinWdh, remWinHgt) {
		var scrLeft = parseInt((screen.width / 2) -  (remWinWdh / 2));
		var scrTop  = parseInt((screen.height / 2) -  (remWinHgt / 2));
        
		var helpwin =  open(filename, windowname, 'width=' + remWinWdh + ',height=' + remWinHgt + ',left=' + scrLeft + ',top=' + scrTop + 'hotkeys=no,scrollbars=yes,resizable=yes');
		popupFollowUp(filename, helpwin);
	}
	function popupFollowUp(filename, win) {
		console.log("popup window object:");
		console.log(win);
		if (!win || win.closed || (typeof (win.closed))=='undefined' || (typeof (win.focus))=='undefined' || !(win.innerHeight) || !(win.innerHeight > 0)) {
			printUnscreened("Error: Browser blocked the popup.  Please allow popups for skotos.net or disable your popup blocker to use them.", "client error");
		} else {
			win.focus();
		}
	}
//-----Style code
	function setActiveStyleSheet(title) {
		var i, a, main;
		for(i=0; (a = document.getElementsByTagName("link")[i]); i++) {
			if(a.getAttribute("rel").indexOf("style") != -1 && a.getAttribute("title")) {
				a.disabled = true;
				if(a.getAttribute("title") == title) a.disabled = false;
			}
		}
		if (skotosAttributes) {
			applySkotosTheme(skotosAttributes);
		}
	}
	function changecss(cls, key, value) {
		var r = document.styleSheets[0].cssRules ? "cssRules" : "rules";
		for (var i=0; i<document.styleSheets[0][r].length;i++) {
			if (document.styleSheets[0][r][i].selectorText == cls) {
				document.styleSheets[0][r][i].style[key] = value;
			}
		}
	}
	var cssMods = {};
	var modStyleSheet = document.createElement('style');
	modStyleSheet.type = 'text/css';
	function updateCssMods() {
		var text = "";
		for (var name in cssMods) {
			text += name+": {"+cssMods+"} ";
		}
		//Todo: Either abandon this method, or update sheet with text here.
	}
	function setStyle(cssText) {
		var sheet = document.createElement('style');
		sheet.type = 'text/css';
		window.customSheet = sheet;
		(document.head || document.getElementsByTagName('head')[0]).appendChild(sheet);
		return (setStyle = function(cssText, node) {
			noteScrollPos();
			if(!node || node.parentNode !== window.customSheet) {
				node = window.customSheet.appendChild(document.createTextNode(cssText));
			} else {
				node.nodeValue = cssText;
			}
			keepScrollPos();
			return node;
		})(cssText);
	}
//-----Logging Code
	function saveCurrentWindow(plain) {
		var format, contents;
		if (prefs.logging_format=="plain") {
			format = "plain";
			contents = output.innerText;
		} else {
			format = "html";
			contents = output.innerHTML;
		}
		var name = logFilename(format, false);
		if (confirm("Saving logfile as: " + name)) {
			var blob = new Blob([contents], {type: "text/" + format + ";charset=utf-8"});
			saveAs(blob, name);
		}
	}
	function logFilename(format, auto) {
		var now = new Date();
		if (Date.prototype.toISOString) {
			now  = now.toISOString();
		} else {
			now = now.toDateString();
		}
		var serverDesc =  serverCode + "_";
		if(gameCharacter) serverDesc += gameCharacter + "_";
		return serverDesc + now + "_" + (auto?"auto":"") + "log." + (format==="html"?"htm":"txt");
	}

	function getUrlParam(parameter, defaultvalue) {

    	var urlparameter = defaultvalue;

    	if(window.location.href.indexOf(parameter) > -1) {
        	urlparameter = getUrlVars()[parameter];
    	}

    	return urlparameter;
	}

	function getUrlVars() {
    	var vars = {};
    	var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        	vars[key] = value;
    	});
    	return vars;
	}

	window.addEventListener("focus", function(event) {
		document.title = document.title.replace(/\* /g, ' ');
	}, false);


	//-----Initialization Code
	//addEvent(window, 'load', init, false);
	addEvent(window, 'load', init, {once:true});