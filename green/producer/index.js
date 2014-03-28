var zeromq = require ('zmq');
var socketDealer = zeromq.socket('dealer');
socketDealer.connect('tcp://10.100.100.100:30000');
socketDealer.send ('green', zeromq.ZMQ_SNDMORE);
socketDealer.send ('Hello world');