var zmq = require('zmq');
var dsock = zmq.socket('dealer');
var psock = zmq.socket('pub');

dsock.bind('tcp://*:30000', function(err) {
	if (err) console.log(err);
});

psock.bind('tcp://*:30001', function(err) {
	if (err) console.log(err);
});

dsock.on('message', function(msg) {
	psock.send(msg);
});
