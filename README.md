zmq-rainbow
===========

RFC for the Rainbow ZeroMQ ecosystem.

The system consists of three components: the Producer, the Store, and the Subscriber. These are described below:

## Producer

The Producer connects a DEALER socket to the Store's DEALER socket on port `30000`, produces messages (see message format below), and sends them to its DEALER socket.

## Subscriber (PUB)

The Subscriber connects a SUB socket to the Store's PUB socket on port `30001`, subscribes to all channels (subscribe to `""`), and then receives and prints messages from that socket.

## Subscriber (ROUTER)

The Subscriber connects a DEALER socket to the Store's ROUTER socket on port `30002`, specifying the channel to subscribe to as the first message. If this is empty it will receive messages from all channels and print them.

## Store

* The Store manages three sockets, a 'frontend', a 'backend pub', and a 'backend router'.

### frontend

* The 'frontend' socket is a DEALER socket.

* The Store binds the 'frontend' socket to all TCP/IP interfaces on port 30000, i.e. the endpoint "tcp://*:30000".

### backend pub

* The 'backend pub' socket is a PUB socket. This is deprecated and should not be used.

* The Store binds the 'backend pub' socket to all TCP/IP interfaces on port 30001, i.e. the endpoint "tcp://*:30001".

* The Store receives messages from the 'frontend' and sends them to the 'backend pub'.

### backend router

* The 'backend router' socket is a ROUTER socket.

* The Store binds the 'backend router' socket to all TCP/IP interfaces on port 30002, i.e. the endpoint "tcp://*:30002".

* The Store receives messages from the 'frontend' and sends them to the 'backend router' clients matching their channel specification.


## Message format

A message contains two frames:

* `channel`: string representing your communication channel. This will be your team color.
* `content`: arbitrary content of the message.
