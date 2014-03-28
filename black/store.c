//  Store

#include <czmq.h>
#include "rainbow_server.h"

int main (void)
{
//     rainbow_server_test (false);

    rainbow_server_t *self = rainbow_server_new ();
    assert (self);
    rainbow_server_bind (self, "tcp://*:30002");
    while (!zctx_interrupted)
        sleep (1);
    rainbow_server_destroy (&self);
    return 0;
}
