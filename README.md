zmq-rainbow
===========

RFC for the Rainbow ZeroMQ ecosystem.

The system consists of three components: the Producer, the Store, and the Subscriber. These are described below:

## Producer

The Producer connects a DEALER socket to the Store's DEALER socket on port `30000`, produces messages (see message format below), and sends them to its DEALER socket.

## Subscriber

The Subscriber connects a SUB socket to the Store's PUB socket on port `30001`, subscribes to all channels (subscribe to `""`), and then receives and prints messages from that socket.

## Store

* The Store manages two sockets, a 'frontend' and a 'backend'.

* The frontend socket is a DEALER socket.

* The backend socket is a PUB socket.

* The Store binds the frontend socket to all TCP/IP interfaces on port 30000, i.e. the endpoint "tcp://*:30000".

* The Store binds the backend socket to all TCP/IP interfaces on port 30001, i.e. the endpoint "tcp://*:30001".

* The Store then receives messages from the front-end and sends them to the backend.

## Message format

A message contains two frames:

* `channel`: string representing your communication channel. This will be your team color.
* `content`: arbitrary content of the message.
