zmq-rainbow
===========

RFC for the Rainbow ZeroMQ ecosystem.

The system consists of three components: the Producer, the Store, and the Subscriber. These are described below:

## Producer
The Producer connects to the Store on port `30000`, produces messages, and sends them to the aforemetnioned Store.


## Store
The Store will bind a Dealer socket to all available ip addresses (use `*`) on port `30000` and listen to incoming messages.

The Store will bind a Publisher socket on all available ip addresses (use `*`) on port `30001`. This will publish messages received on the Dealer socket.


## Subscriber
The Subscriber connects to the Subscriber on port `30001`, receives messages, and prints them. 


# Message format
Messages contain two frames:

 - `channel`: string representing your communication channel. This will be your team color.
 - `message`: arbitrary content of the message.
