/*  =========================================================================
    rainbow_msg - Rainbow protocol
    
    Codec header for rainbow_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

    * The XML model used for this code generation: rainbow_msg.xml
    * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    =========================================================================
*/

#ifndef __RAINBOW_MSG_H_INCLUDED__
#define __RAINBOW_MSG_H_INCLUDED__

/*  These are the rainbow_msg messages:

    SUBSCRIBE - Subscribe to a channel
        channel             string      Name of channel

    PUBLISH - Publish an event to a channel
        channel             string      Name of channel
        content             frame       Content frame

    DELIVER - Deliver a new event off a channel
        channel             string      Name of channel
        content             frame       Content frame

    WTF - Reject client command with error
        reason              string      Error reason
*/


#define RAINBOW_MSG_SUBSCRIBE               1
#define RAINBOW_MSG_PUBLISH                 2
#define RAINBOW_MSG_DELIVER                 3
#define RAINBOW_MSG_WTF                     4

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _rainbow_msg_t rainbow_msg_t;

//  @interface
//  Create a new rainbow_msg
rainbow_msg_t *
    rainbow_msg_new (int id);

//  Destroy the rainbow_msg
void
    rainbow_msg_destroy (rainbow_msg_t **self_p);

//  Parse a rainbow_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. If the socket type is
//  ZMQ_ROUTER, then parses the first frame as a routing_id. Destroys msg
//  and nullifies the msg refernce.
rainbow_msg_t *
    rainbow_msg_decode (zmsg_t **msg_p, int socket_type);

//  Encode rainbow_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
//  If the socket_type is ZMQ_ROUTER, then stores the routing_id as the
//  first frame of the resulting message.
zmsg_t *
    rainbow_msg_encode (rainbow_msg_t *self, int socket_type);

//  Receive and parse a rainbow_msg from the socket. Returns new object, 
//  or NULL if error. Will block if there's no message waiting.
rainbow_msg_t *
    rainbow_msg_recv (void *input);

//  Receive and parse a rainbow_msg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.
rainbow_msg_t *
    rainbow_msg_recv_nowait (void *input);

//  Send the rainbow_msg to the output, and destroy it
int
    rainbow_msg_send (rainbow_msg_t **self_p, void *output);

//  Send the rainbow_msg to the output, and do not destroy it
int
    rainbow_msg_send_again (rainbow_msg_t *self, void *output);

//  Send the SUBSCRIBE to the output in one step
int
    rainbow_msg_send_subscribe (void *output,
        const char *channel);
    
//  Send the PUBLISH to the output in one step
int
    rainbow_msg_send_publish (void *output,
        const char *channel,
        zframe_t *content);
    
//  Send the DELIVER to the output in one step
int
    rainbow_msg_send_deliver (void *output,
        const char *channel,
        zframe_t *content);
    
//  Send the WTF to the output in one step
int
    rainbow_msg_send_wtf (void *output,
        const char *reason);
    
//  Duplicate the rainbow_msg message
rainbow_msg_t *
    rainbow_msg_dup (rainbow_msg_t *self);

//  Print contents of message to stdout
void
    rainbow_msg_dump (rainbow_msg_t *self);

//  Get/set the message routing id
zframe_t *
    rainbow_msg_routing_id (rainbow_msg_t *self);
void
    rainbow_msg_set_routing_id (rainbow_msg_t *self, zframe_t *routing_id);

//  Get the rainbow_msg id and printable command
int
    rainbow_msg_id (rainbow_msg_t *self);
void
    rainbow_msg_set_id (rainbow_msg_t *self, int id);
const char *
    rainbow_msg_command (rainbow_msg_t *self);

//  Get/set the channel field
const char *
    rainbow_msg_channel (rainbow_msg_t *self);
void
    rainbow_msg_set_channel (rainbow_msg_t *self, const char *format, ...);

//  Get a copy of the content field
zframe_t *
    rainbow_msg_content (rainbow_msg_t *self);
//  Get the content field and transfer ownership to caller
zframe_t *
    rainbow_msg_get_content (rainbow_msg_t *self);
//  Set the content field, transferring ownership from caller
void
    rainbow_msg_set_content (rainbow_msg_t *self, zframe_t **frame_p);

//  Get/set the reason field
const char *
    rainbow_msg_reason (rainbow_msg_t *self);
void
    rainbow_msg_set_reason (rainbow_msg_t *self, const char *format, ...);

//  Self test of this class
int
    rainbow_msg_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
