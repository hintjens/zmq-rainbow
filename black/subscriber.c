//  Subscriber

#include <czmq.h>

int main (int argc, char *argv [])
{
    zctx_t *ctx = zctx_new ();
    void *subscriber = zsocket_new (ctx, ZMQ_SUB);
    char *endpoint = "localhost";
    if (argc > 1)
        endpoint = argv [1];
    zsocket_connect (subscriber, "tcp://%s:30001", endpoint);
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
