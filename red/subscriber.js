var zmq = require('zmq');
var sub = zmq.socket('sub');

sub.connect('tcp://127.0.0.1:30001');
sub.subscribe('');

sub.on('message', function(msg) {
	console.log(msg.toString());
});
