var zmq = require('zmq');
var socket = zmq.socket('dealer');

socket.connect('tcp://' + '10.100.100.116' + ':30002');

socket.on('message', function (channel, content) {
	console.log(new Date(), channel.toString(), content.toString());
});

socket.send('brown');