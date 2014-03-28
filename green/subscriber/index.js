var zeromq = require ('zmq');
var socketSub = zeromq.socket('sub');
socketSub.connect('tcp://10.100.100.100:30001');
socketSub.subscribe('');

socketSub.on ('message', function (channel, msg)
{
    console.log (msg.toString());
});