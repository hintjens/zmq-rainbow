zmq-rainbow
===========

RFC for the Rainbow ZeroMQ ecosystem.

The system consists of three components: the Producer, the Store, and the Subscriber. These are described below:

## Producer
The Producer connects a Dealer socket to the Store's Dealer socket on port `30000`, produces messages, and sends them to the aforementioned Store.


## Store
The Store binds a Dealer socket to all available ip addresses (`*`) on port `30000` and listen to incoming messages.

The Store binds a Publisher socket on all available ip addresses (`*`) on port `30001`. This Publisher will publish messages received on the Dealer socket.


## Subscriber
The Subscriber connects to the Store on port `30001` using a subscribe socket, receives messages, and prints their channel and message fields. 


# Message format
Messages contain two frames:

 - `channel`: string representing your communication channel. This will be your team color.
 - `message`: arbitrary content of the message.
