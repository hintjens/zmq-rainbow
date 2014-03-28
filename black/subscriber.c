//  Subscriber

#include <czmq.h>

int main (void)
{
    zctx_t *ctx = zctx_new ();
    void *subscriber = zsocket_new (ctx, ZMQ_SUB);
    zsocket_connect (subscriber, "tcp://localhost:30001");
    zsocket_set_subscribe (subscriber, "");
    
    while (true) {
        zmsg_t *msg = zmsg_recv (subscriber);
        if (!msg)
            break;
        zmsg_dump (msg);
        zmsg_destroy (&msg);
    }
    zctx_destroy (&ctx);
    return 0;
}
