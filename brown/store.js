var zmq = require('zmq')
var address = 'tcp://*:30000';
var pubAddress = 'tcp://*:30001';
var socket = zmq.socket('dealer');
var pubSocket = zmq.socket('pub');

socket.bind(address, function(err) {
	if (err){
		throw err;
	}
	pubSocket.bind(pubAddress, function(err) {
		if (err){
			throw err;
		}
		socket.on('message', function (channel, content) {
			console.log(new Date(), channel.toString(), content.toString());
			pubSocket.send([channel, content]);
		});
	});
});