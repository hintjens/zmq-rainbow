//  Producer

#include <czmq.h>

int main (int argc, char *argv [])
{
    zctx_t *ctx = zctx_new ();
    
    void *client = zsocket_new (ctx, ZMQ_DEALER);
    char *endpoint = "localhost";
    if (argc > 1)
        endpoint = argv [1];
    zsocket_connect (client, "tcp://%s:30000", endpoint);

    while (!zctx_interrupted) {
        zmsg_t *msg = zmsg_new ();
        zmsg_addstr (msg, "black");
        zmsg_addstr (msg, "Hello, World!");
        zmsg_send (&msg, client);
        printf (".");
        fflush (stdout);
        sleep (1);
    }
    zctx_destroy (&ctx);
    return 0;
}
