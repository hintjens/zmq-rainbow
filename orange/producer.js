var zmq = require('zmq');
var sock = zmq.socket('dealer');

sock.connect('tcp://127.0.0.1:30000');

var publish = function(channel, message) {
	sock.send(channel, zmq.ZMQ_SNDMORE);
	sock.send(message);
};

publish('orange', 'hello world');
