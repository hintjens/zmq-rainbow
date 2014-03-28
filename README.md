zmq-rainbow
===========

RFC for the Rainbow ZeroMQ ecosystem.

The system consists of three components: the Producer, the Store, and the Subscriber. These are described below:

## Producer
The Producer connects to the Store on port `30000`, produces messages, and sends them.


## Store
The Store accepts incoming messages on port `30000` and publishes events on port `30001`.


## Subscriber
The Subscriber connects to the Subscriber on port `30001`, receives messages, and prints them. 


# Message format
Messages contain two frames:

 - `channel`: string representing your communication channel. This will be your team color.
 - `message`: arbitrary content of the message.
