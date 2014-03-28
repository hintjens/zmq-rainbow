var zmq = require('zmq');
var dsock = zmq.socket('dealer');

dsock.connect('tcp://127.0.0.1:30000');

var args = process.argv;
args.shift();
args.shift();
dsock.send(args.join(' '));

setTimeout(function() {process.exit(0)},400);
