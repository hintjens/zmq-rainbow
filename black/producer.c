//  Producer

#include <czmq.h>

int main (void)
{
    zctx_t *ctx = zctx_new ();
    
    void *client = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_connect (client, "tcp://localhost:30000");

    while (true) {
        zmsg_t *msg = zmsg_new ();
        zmsg_addstr (msg, "black");
        zmsg_addstr (msg, "Hello, World!");
        zmsg_send (&msg, client);
        sleep (1);
    }
    zctx_destroy (&ctx);
    return 0;
}
