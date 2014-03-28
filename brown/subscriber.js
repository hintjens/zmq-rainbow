var zmq = require('zmq');
var socket = zmq.socket('sub');
var ips = require('./ips');

socket.connect('tcp://' + ips.black + ':30001');
socket.subscribe('');
socket.on('message', function (channel, content) {
	console.log(new Date(), channel.toString(), content.toString());
});
