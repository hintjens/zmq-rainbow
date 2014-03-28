var zmq = require('zmq')
var address = 'tcp://*:30000';
var pubAddress = 'tcp://*:30001';
var routerAddress = 'tcp://*:30002';
var socket = zmq.socket('dealer');
var pubSocket = zmq.socket('pub');
var routerSocket = zmq.socket('router');
var clients = {};

socket.bind(address, function(err) {
	if (err){
		throw err;
	}
	pubSocket.bind(pubAddress, function(err) {
		if (err){
			throw err;
		}
		routerSocket.bind(routerAddress, function(err) {
			if (err) {
				throw err;
			}
			routerSocket.on('message', function(envelope, data) {
				console.log(socket.identity + ': received ' + envelope.toString() + ' - ' + data.toString());
				clients[data.toString()] = envelope;
			});

			socket.on('message', function (channel, content) {
				console.log(new Date(), channel.toString(), content.toString());
				pubSocket.send([channel, content]);
				Object.keys(clients).forEach(function(channelToListenTo){
					if(channel.toString().indexOf(channelToListenTo) === 0) {
						var envelope = clients[channelToListenTo];
						console.log('send', envelope.toString(), channel.toString(), content.toString());
						routerSocket.send([envelope, channel, content]);
					}
				});
			});
		});
	});
});