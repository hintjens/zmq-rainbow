var zmq = require('zmq');
var socket = zmq.socket('dealer');
var ips = require('./ips');
socket.connect('tcp://' + ips.orange + ':30000');

setInterval(function(){
	console.log('sending work');
	socket.send(['brown', 'hello world']);
}, 500);