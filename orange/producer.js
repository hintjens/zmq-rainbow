var zmq = require('zmq');
var sock = zmq.socket('dealer');


module.exports = function(addr) {
	sock.connect('tcp://'+addr+':30000');

	return function(channel, message) {
		sock.send(channel, zmq.ZMQ_SNDMORE);
		sock.send(message);
	};
};
