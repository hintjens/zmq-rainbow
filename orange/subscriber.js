var zmq = require('zmq');
var sock = zmq.socket('sub');

module.exports = function(addr) {
	sock.connect('tcp://'+addr+':30001');

	return function sub(channel, fn) {
		if (typeof channel === 'function') return sub(null, channel);
		sock.subscribe(channel || '');
		sock.on('message', function(channel, message) {
			fn(channel, message);
		});
	};
};