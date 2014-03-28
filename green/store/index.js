var zeromq = require ('zmq');
var socketDealer = zeromq.socket('dealer');
var socketPublisher = zeromq.socket('pub');
socketDealer.bind('tcp://*:30000');
socketPublisher.bind('tcp://*:30001');

socketDealer.on ("message", function (channel, msg)
{
    console.log (msg.toString());
    socketPublisher.send (channel.toString(), zeromq.ZMQ_SNDMORE);
    socketPublisher.send (msg.toString());
});