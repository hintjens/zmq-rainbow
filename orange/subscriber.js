var zmq = require('zmq');
var sock = zmq.socket('sub');

sock.connect('tcp://127.0.0.1:30001');
sock.subscribe('');

sock.on('message', function(channel, message) {
	console.log('onmessage', channel.toString(), message.toString());
});