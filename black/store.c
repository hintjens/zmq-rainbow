//  Store

#include <czmq.h>

int main (void)
{
    zctx_t *ctx = zctx_new ();
    
    void *frontend = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_bind (frontend, "tcp://*:30000");
    
    void *backend = zsocket_new (ctx, ZMQ_PUB);
    zsocket_bind (backend, "tcp://*:30001");
    
    while (true) {
        zmsg_t *msg = zmsg_recv (frontend);
        if (!msg)
            break;
        zmsg_dump (msg);
        zmsg_send (&msg, backend);
    }
    zctx_destroy (&ctx);
    return 0;
}
