/*  =========================================================================
    rainbow_msg - Rainbow protocol

    Codec class for rainbow_msg.

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

/*
@header
    rainbow_msg - Rainbow protocol
@discuss
@end
*/

#include <czmq.h>
#include "./rainbow_msg.h"

//  Structure of our class

struct _rainbow_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  rainbow_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char *channel;                      //  Name of channel
    zframe_t *content;                  //  Content frame
    char *reason;                       //  Error reason
};

//  --------------------------------------------------------------------------
//  Network data encoding macros

//  Put a block of octets to the frame
#define PUT_OCTETS(host,size) { \
    memcpy (self->needle, (host), size); \
    self->needle += size; \
}

//  Get a block of octets from the frame
#define GET_OCTETS(host,size) { \
    if (self->needle + size > self->ceiling) \
        goto malformed; \
    memcpy ((host), self->needle, size); \
    self->needle += size; \
}

//  Put a 1-byte number to the frame
#define PUT_NUMBER1(host) { \
    *(byte *) self->needle = (host); \
    self->needle++; \
}

//  Put a 2-byte number to the frame
#define PUT_NUMBER2(host) { \
    self->needle [0] = (byte) (((host) >> 8)  & 255); \
    self->needle [1] = (byte) (((host))       & 255); \
    self->needle += 2; \
}

//  Put a 4-byte number to the frame
#define PUT_NUMBER4(host) { \
    self->needle [0] = (byte) (((host) >> 24) & 255); \
    self->needle [1] = (byte) (((host) >> 16) & 255); \
    self->needle [2] = (byte) (((host) >> 8)  & 255); \
    self->needle [3] = (byte) (((host))       & 255); \
    self->needle += 4; \
}

//  Put a 8-byte number to the frame
#define PUT_NUMBER8(host) { \
    self->needle [0] = (byte) (((host) >> 56) & 255); \
    self->needle [1] = (byte) (((host) >> 48) & 255); \
    self->needle [2] = (byte) (((host) >> 40) & 255); \
    self->needle [3] = (byte) (((host) >> 32) & 255); \
    self->needle [4] = (byte) (((host) >> 24) & 255); \
    self->needle [5] = (byte) (((host) >> 16) & 255); \
    self->needle [6] = (byte) (((host) >> 8)  & 255); \
    self->needle [7] = (byte) (((host))       & 255); \
    self->needle += 8; \
}

//  Get a 1-byte number from the frame
#define GET_NUMBER1(host) { \
    if (self->needle + 1 > self->ceiling) \
        goto malformed; \
    (host) = *(byte *) self->needle; \
    self->needle++; \
}

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) \
        goto malformed; \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
}

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) \
        goto malformed; \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
}

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) \
        goto malformed; \
    (host) = ((uint64_t) (self->needle [0]) << 56) \
           + ((uint64_t) (self->needle [1]) << 48) \
           + ((uint64_t) (self->needle [2]) << 40) \
           + ((uint64_t) (self->needle [3]) << 32) \
           + ((uint64_t) (self->needle [4]) << 24) \
           + ((uint64_t) (self->needle [5]) << 16) \
           + ((uint64_t) (self->needle [6]) << 8) \
           +  (uint64_t) (self->needle [7]); \
    self->needle += 8; \
}

//  Put a string to the frame
#define PUT_STRING(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER1 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a string from the frame
#define GET_STRING(host) { \
    size_t string_size; \
    GET_NUMBER1 (string_size); \
    if (self->needle + string_size > (self->ceiling)) \
        goto malformed; \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}

//  Put a long string to the frame
#define PUT_LONGSTR(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER4 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a long string from the frame
#define GET_LONGSTR(host) { \
    size_t string_size; \
    GET_NUMBER4 (string_size); \
    if (self->needle + string_size > (self->ceiling)) \
        goto malformed; \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}


//  --------------------------------------------------------------------------
//  Create a new rainbow_msg

rainbow_msg_t *
rainbow_msg_new (int id)
{
    rainbow_msg_t *self = (rainbow_msg_t *) zmalloc (sizeof (rainbow_msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the rainbow_msg

void
rainbow_msg_destroy (rainbow_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        rainbow_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->channel);
        zframe_destroy (&self->content);
        free (self->reason);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Parse a rainbow_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. If the socket type is
//  ZMQ_ROUTER, then parses the first frame as a routing_id. Destroys msg
//  and nullifies the msg refernce.

rainbow_msg_t *
rainbow_msg_decode (zmsg_t **msg_p, int socket_type)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    rainbow_msg_t *self = rainbow_msg_new (0);
    //  If message came from a router socket, first frame is routing_id
    if (socket_type == ZMQ_ROUTER) {
        self->routing_id = zmsg_pop (msg);
        //  If message was not valid, forget about it
        if (!self->routing_id || !zmsg_next (msg)) {
            rainbow_msg_destroy (&self);
            return (NULL);      //  Malformed or empty
        }
    }
    //  Read and parse command in frame
    zframe_t *frame = zmsg_pop (msg);
    if (!frame) 
        goto empty;             //  Malformed or empty

    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 0))
        goto empty;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case RAINBOW_MSG_SUBSCRIBE:
            GET_STRING (self->channel);
            break;

        case RAINBOW_MSG_PUBLISH:
            GET_STRING (self->channel);
            {
                //  Get next frame, leave current untouched
                zframe_t *content = zmsg_pop (msg);
                if (!content)
                    goto malformed;
                self->content = content;
            }
            break;

        case RAINBOW_MSG_DELIVER:
            GET_STRING (self->channel);
            {
                //  Get next frame, leave current untouched
                zframe_t *content = zmsg_pop (msg);
                if (!content)
                    goto malformed;
                self->content = content;
            }
            break;

        case RAINBOW_MSG_WTF:
            GET_STRING (self->reason);
            break;

        default:
            goto malformed;
    }
    //  Successful return
    zframe_destroy (&frame);
    zmsg_destroy (msg_p);
    return self;

    //  Error returns
    malformed:
        printf ("E: malformed message '%d'\n", self->id);
    empty:
        zframe_destroy (&frame);
        zmsg_destroy (msg_p);
        rainbow_msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Receive and parse a rainbow_msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

rainbow_msg_t *
rainbow_msg_recv (void *input)
{
    assert (input);
    zmsg_t *msg = zmsg_recv (input);
    return rainbow_msg_decode (&msg, zsocket_type (input));
}


//  --------------------------------------------------------------------------
//  Receive and parse a rainbow_msg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.

rainbow_msg_t *
rainbow_msg_recv_nowait (void *input)
{
    assert (input);
    zmsg_t *msg = zmsg_recv_nowait (input);
    return rainbow_msg_decode (&msg, zsocket_type (input));
}


//  Encode rainbow_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
//  If the socket_type is ZMQ_ROUTER, then stores the routing_id as the
//  first frame of the resulting message.

zmsg_t *
rainbow_msg_encode (rainbow_msg_t *self, int socket_type)
{
    assert (self);
    zmsg_t *msg = zmsg_new ();

    //  If we're sending to a ROUTER, send the routing_id first
    if (socket_type == ZMQ_ROUTER)
        zmsg_prepend (msg, &self->routing_id);
        
    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case RAINBOW_MSG_SUBSCRIBE:
            //  channel is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->channel)
                frame_size += strlen (self->channel);
            break;
            
        case RAINBOW_MSG_PUBLISH:
            //  channel is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->channel)
                frame_size += strlen (self->channel);
            break;
            
        case RAINBOW_MSG_DELIVER:
            //  channel is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->channel)
                frame_size += strlen (self->channel);
            break;
            
        case RAINBOW_MSG_WTF:
            //  reason is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->reason)
                frame_size += strlen (self->reason);
            break;
            
        default:
            printf ("E: bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 0);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case RAINBOW_MSG_SUBSCRIBE:
            if (self->channel) {
                PUT_STRING (self->channel);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case RAINBOW_MSG_PUBLISH:
            if (self->channel) {
                PUT_STRING (self->channel);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case RAINBOW_MSG_DELIVER:
            if (self->channel) {
                PUT_STRING (self->channel);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case RAINBOW_MSG_WTF:
            if (self->reason) {
                PUT_STRING (self->reason);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        rainbow_msg_destroy (&self);
        return NULL;
    }
    //  Now send any frame fields, in order
    if (self->id == RAINBOW_MSG_PUBLISH) {
        //  If content isn't set, send an empty frame
        if (!self->content)
            self->content = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->content)) {
            zmsg_destroy (&msg);
            rainbow_msg_destroy (&self);
            return NULL;
        }
    }
    //  Now send any frame fields, in order
    if (self->id == RAINBOW_MSG_DELIVER) {
        //  If content isn't set, send an empty frame
        if (!self->content)
            self->content = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->content)) {
            zmsg_destroy (&msg);
            rainbow_msg_destroy (&self);
            return NULL;
        }
    }
    //  Destroy rainbow_msg object
    rainbow_msg_destroy (&self);
    return msg;

}

//  --------------------------------------------------------------------------
//  Send the rainbow_msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
rainbow_msg_send (rainbow_msg_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    rainbow_msg_t *self = *self_p;
    zmsg_t *msg = rainbow_msg_encode (self, zsocket_type (output));
    if (msg && zmsg_send (&msg, output) == 0)
        return 0;
    else
        return -1;              //  Failed to encode, or send
}


//  --------------------------------------------------------------------------
//  Send the rainbow_msg to the output, and do not destroy it

int
rainbow_msg_send_again (rainbow_msg_t *self, void *output)
{
    assert (self);
    assert (output);
    self = rainbow_msg_dup (self);
    return rainbow_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the SUBSCRIBE to the socket in one step

int
rainbow_msg_send_subscribe (
    void *output,
    const char *channel)
{
    rainbow_msg_t *self = rainbow_msg_new (RAINBOW_MSG_SUBSCRIBE);
    rainbow_msg_set_channel (self, channel);
    return rainbow_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the PUBLISH to the socket in one step

int
rainbow_msg_send_publish (
    void *output,
    const char *channel,
    zframe_t *content)
{
    rainbow_msg_t *self = rainbow_msg_new (RAINBOW_MSG_PUBLISH);
    rainbow_msg_set_channel (self, channel);
    zframe_t *content_copy = zframe_dup (content);
    rainbow_msg_set_content (self, &content_copy);
    return rainbow_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DELIVER to the socket in one step

int
rainbow_msg_send_deliver (
    void *output,
    const char *channel,
    zframe_t *content)
{
    rainbow_msg_t *self = rainbow_msg_new (RAINBOW_MSG_DELIVER);
    rainbow_msg_set_channel (self, channel);
    zframe_t *content_copy = zframe_dup (content);
    rainbow_msg_set_content (self, &content_copy);
    return rainbow_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the WTF to the socket in one step

int
rainbow_msg_send_wtf (
    void *output,
    const char *reason)
{
    rainbow_msg_t *self = rainbow_msg_new (RAINBOW_MSG_WTF);
    rainbow_msg_set_reason (self, reason);
    return rainbow_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the rainbow_msg message

rainbow_msg_t *
rainbow_msg_dup (rainbow_msg_t *self)
{
    if (!self)
        return NULL;
        
    rainbow_msg_t *copy = rainbow_msg_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);

    switch (self->id) {
        case RAINBOW_MSG_SUBSCRIBE:
            copy->channel = self->channel? strdup (self->channel): NULL;
            break;

        case RAINBOW_MSG_PUBLISH:
            copy->channel = self->channel? strdup (self->channel): NULL;
            copy->content = self->content? zframe_dup (self->content): NULL;
            break;

        case RAINBOW_MSG_DELIVER:
            copy->channel = self->channel? strdup (self->channel): NULL;
            copy->content = self->content? zframe_dup (self->content): NULL;
            break;

        case RAINBOW_MSG_WTF:
            copy->reason = self->reason? strdup (self->reason): NULL;
            break;

    }
    return copy;
}



//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
rainbow_msg_dump (rainbow_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case RAINBOW_MSG_SUBSCRIBE:
            puts ("SUBSCRIBE:");
            if (self->channel)
                printf ("    channel='%s'\n", self->channel);
            else
                printf ("    channel=\n");
            break;
            
        case RAINBOW_MSG_PUBLISH:
            puts ("PUBLISH:");
            if (self->channel)
                printf ("    channel='%s'\n", self->channel);
            else
                printf ("    channel=\n");
            printf ("    content={\n");
            if (self->content)
                zframe_print (self->content, NULL);
            else
                printf ("(NULL)\n");
            printf ("    }\n");
            break;
            
        case RAINBOW_MSG_DELIVER:
            puts ("DELIVER:");
            if (self->channel)
                printf ("    channel='%s'\n", self->channel);
            else
                printf ("    channel=\n");
            printf ("    content={\n");
            if (self->content)
                zframe_print (self->content, NULL);
            else
                printf ("(NULL)\n");
            printf ("    }\n");
            break;
            
        case RAINBOW_MSG_WTF:
            puts ("WTF:");
            if (self->reason)
                printf ("    reason='%s'\n", self->reason);
            else
                printf ("    reason=\n");
            break;
            
    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
rainbow_msg_routing_id (rainbow_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
rainbow_msg_set_routing_id (rainbow_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the rainbow_msg id

int
rainbow_msg_id (rainbow_msg_t *self)
{
    assert (self);
    return self->id;
}

void
rainbow_msg_set_id (rainbow_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
rainbow_msg_command (rainbow_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case RAINBOW_MSG_SUBSCRIBE:
            return ("SUBSCRIBE");
            break;
        case RAINBOW_MSG_PUBLISH:
            return ("PUBLISH");
            break;
        case RAINBOW_MSG_DELIVER:
            return ("DELIVER");
            break;
        case RAINBOW_MSG_WTF:
            return ("WTF");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the channel field

const char *
rainbow_msg_channel (rainbow_msg_t *self)
{
    assert (self);
    return self->channel;
}

void
rainbow_msg_set_channel (rainbow_msg_t *self, const char *format, ...)
{
    //  Format channel from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->channel);
    self->channel = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the content field without transferring ownership

zframe_t *
rainbow_msg_content (rainbow_msg_t *self)
{
    assert (self);
    return self->content;
}

//  Get the content field and transfer ownership to caller

zframe_t *
rainbow_msg_get_content (rainbow_msg_t *self)
{
    zframe_t *content = self->content;
    self->content = NULL;
    return content;
}

//  Set the content field, transferring ownership from caller

void
rainbow_msg_set_content (rainbow_msg_t *self, zframe_t **frame_p)
{
    assert (self);
    assert (frame_p);
    zframe_destroy (&self->content);
    self->content = *frame_p;
    *frame_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the reason field

const char *
rainbow_msg_reason (rainbow_msg_t *self)
{
    assert (self);
    return self->reason;
}

void
rainbow_msg_set_reason (rainbow_msg_t *self, const char *format, ...)
{
    //  Format reason from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->reason);
    self->reason = zsys_vprintf (format, argptr);
    va_end (argptr);
}



//  --------------------------------------------------------------------------
//  Selftest

int
rainbow_msg_test (bool verbose)
{
    printf (" * rainbow_msg: ");

    //  @selftest
    //  Simple create/destroy test
    rainbow_msg_t *self = rainbow_msg_new (0);
    assert (self);
    rainbow_msg_destroy (&self);

    //  Create pair of sockets we can send through
    zctx_t *ctx = zctx_new ();
    assert (ctx);

    void *output = zsocket_new (ctx, ZMQ_DEALER);
    assert (output);
    zsocket_bind (output, "inproc://selftest");

    void *input = zsocket_new (ctx, ZMQ_ROUTER);
    assert (input);
    zsocket_connect (input, "inproc://selftest");
    
    //  Encode/send/decode and verify each message type
    int instance;
    rainbow_msg_t *copy;
    self = rainbow_msg_new (RAINBOW_MSG_SUBSCRIBE);
    
    //  Check that _dup works on empty message
    copy = rainbow_msg_dup (self);
    assert (copy);
    rainbow_msg_destroy (&copy);

    rainbow_msg_set_channel (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    rainbow_msg_send_again (self, output);
    rainbow_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = rainbow_msg_recv (input);
        assert (self);
        assert (rainbow_msg_routing_id (self));
        
        assert (streq (rainbow_msg_channel (self), "Life is short but Now lasts for ever"));
        rainbow_msg_destroy (&self);
    }
    self = rainbow_msg_new (RAINBOW_MSG_PUBLISH);
    
    //  Check that _dup works on empty message
    copy = rainbow_msg_dup (self);
    assert (copy);
    rainbow_msg_destroy (&copy);

    rainbow_msg_set_channel (self, "Life is short but Now lasts for ever");
    zframe_t *publish_content = zframe_new ("Captcha Diem", 12);
    rainbow_msg_set_content (self, &publish_content);
    //  Send twice from same object
    rainbow_msg_send_again (self, output);
    rainbow_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = rainbow_msg_recv (input);
        assert (self);
        assert (rainbow_msg_routing_id (self));
        
        assert (streq (rainbow_msg_channel (self), "Life is short but Now lasts for ever"));
        assert (zframe_streq (rainbow_msg_content (self), "Captcha Diem"));
        rainbow_msg_destroy (&self);
    }
    self = rainbow_msg_new (RAINBOW_MSG_DELIVER);
    
    //  Check that _dup works on empty message
    copy = rainbow_msg_dup (self);
    assert (copy);
    rainbow_msg_destroy (&copy);

    rainbow_msg_set_channel (self, "Life is short but Now lasts for ever");
    zframe_t *deliver_content = zframe_new ("Captcha Diem", 12);
    rainbow_msg_set_content (self, &deliver_content);
    //  Send twice from same object
    rainbow_msg_send_again (self, output);
    rainbow_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = rainbow_msg_recv (input);
        assert (self);
        assert (rainbow_msg_routing_id (self));
        
        assert (streq (rainbow_msg_channel (self), "Life is short but Now lasts for ever"));
        assert (zframe_streq (rainbow_msg_content (self), "Captcha Diem"));
        rainbow_msg_destroy (&self);
    }
    self = rainbow_msg_new (RAINBOW_MSG_WTF);
    
    //  Check that _dup works on empty message
    copy = rainbow_msg_dup (self);
    assert (copy);
    rainbow_msg_destroy (&copy);

    rainbow_msg_set_reason (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    rainbow_msg_send_again (self, output);
    rainbow_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = rainbow_msg_recv (input);
        assert (self);
        assert (rainbow_msg_routing_id (self));
        
        assert (streq (rainbow_msg_reason (self), "Life is short but Now lasts for ever"));
        rainbow_msg_destroy (&self);
    }

    zctx_destroy (&ctx);
    //  @end

    printf ("OK\n");
    return 0;
}
