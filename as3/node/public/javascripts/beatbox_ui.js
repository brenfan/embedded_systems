"use strict";
// Client-side interactions with the browser.

// Web sockets: automatically establish a socket with the server
var socket = io.connect();

// Make connection to server when web page is fully loaded.
$(document).ready(function() {
	// Make the text-entry box have keyboard focus.
	$('#send-command').focus();

	// Allow sending the form (pressing enter).
	$('#send-form').submit(function() {
		readUserInput();

		// Return false to show we have handleded it.
		return false;
	});

	// Handle data coming back from the server
	socket.on('daAnswer', function(result) {
		$('#messages').append(divMessage(result));
	});

	socket.on('daError', function(result) {
		var msg = divMessage('SERVER ERROR: ' + result);
		$('#messages').append(msg);
	});

});
