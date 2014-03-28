var zmq = require('zmq');
var dsock = zmq.socket('dealer');
var psock = zmq.socket('pub');

dsock.bind('tcp://127.0.0.1:30000', function(err) {
	if (err) console.log(err);
});

psock.bind('tcp://127.0.0.1:30001', function(err) {
	if (err) console.log(err);
});

dsock.on('bind', function() {
	console.log('Bound');
});

psock.on('bind', function() {
	console.log('Bound');
});


setTimeout(function() {
	psock.send('HIIIIII');
},1000);

dsock.on('message', function(msg) {
	console.log(msg);
	psock.send(msg);
});
