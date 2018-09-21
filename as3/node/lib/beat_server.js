/*
 * Respond to commands over a websocket to do math
 */

var socketio = require('socket.io');
var io;

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');

	io.sockets.on('connection', function(socket) {

		handleCommand(socket);
	});

};

function handleCommand(socket) {
	var errorTimer = setTimeout(function() {
		socket.emit("daError", "Oops: User too slow at sending first command.");
	}, 5000);

	socket.on('sendData', function(data) {

	});

	// ... add more commands here...
	socket.on('daSub', function(data){
		// ...
	});
};
