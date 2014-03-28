/*  =========================================================================
    rainbow_server - Rainbow server

    =========================================================================
*/

/*
@header
    Description of class for man page.
@discuss
    Detailed discussion of the class, if any.
@end
*/

#include <czmq.h>
#include "rainbow_msg.h"
#include "rainbow_server.h"


//  ---------------------------------------------------------------------
//  Forward declarations for the two main classes we use here

typedef struct _server_t server_t;
typedef struct _client_t client_t;

//  This structure defines the context for each running server. Store
//  whatever properties and structures you need for the server.

struct _server_t {
    //  Add any properties you need here
    int filler;             //  Structure can't be empty (pedantic)
};

//  Allocate properties and structures for a new server instance.
//  Return 0 if OK, or -1 if there was an error.

static int
server_initialize (server_t *self)
{
    //  Construct properties here
    return 0;
}

//  Free properties and structures for a server instance

static void
server_terminate (server_t *self)
{
    //  Destroy properties here
}


//  ---------------------------------------------------------------------
//  This structure defines the state for each client connection. It will
//  be passed to each action in the 'self' argument.

struct _client_t {
    //  These properties must always be present in the client_t
    server_t *server;           //  Reference to parent server
    rainbow_msg_t *request;     //  Last received request
    rainbow_msg_t *reply;       //  Reply to send out, if any
    //  These properties are specific for this application
};

//  Allocate properties and structures for a new client connection
//  Return 0 if OK, or -1 if there was an error.

static int
client_initialize (client_t *self)
{
    //  Construct properties here
    return 0;
}

//  Free properties and structures for a client connection

static void
client_terminate (client_t *self)
{
    //  Destroy properties here
}

//  Include the generated server engine
#include "rainbow_server_engine.h"




//  --------------------------------------------------------------------------
//  subscribe_the_client
//

static void
subscribe_the_client (client_t *self)
{

}


//  --------------------------------------------------------------------------
//  publish_the_event
//

static void
publish_the_event (client_t *self)
{

}


//  --------------------------------------------------------------------------
//  Selftest

void
rainbow_server_test (bool verbose)
{
    printf (" * rainbow_server: \n");
    
    //  @selftest
    zctx_t *ctx = zctx_new ();

    rainbow_server_t *self = rainbow_server_new ();
    assert (self);
    rainbow_server_bind (self, "tcp://127.0.0.1:30002");

    void *dealer = zsocket_new (ctx, ZMQ_DEALER);
    assert (dealer);
    zsocket_set_rcvtimeo (dealer, 2000);
    zsocket_connect (dealer, "tcp://localhost:30002");

    rainbow_msg_t *request, *reply;
    request = rainbow_msg_new (RAINBOW_MSG_SUBSCRIBE);
    rainbow_msg_set_channel (request, "black");
    rainbow_msg_send (&request, dealer);
    reply = rainbow_msg_recv (dealer);
    assert (reply);
    assert (rainbow_msg_id (reply) == RAINBOW_MSG_WTF);
    rainbow_msg_destroy (&reply);
    rainbow_server_destroy (&self);

    zctx_destroy (&ctx);
    //  @end

    //  No clean way to wait for a background thread to exit
    //  Under valgrind this will randomly show as leakage
    //  Reduce this by giving server thread time to exit
    zclock_sleep (200);
    printf ("OK\n");
}
