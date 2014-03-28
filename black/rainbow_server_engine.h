/*  =========================================================================
    rainbow_server_engine - Rainbow server engine

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

    * The XML model used for this code generation: rainbow_server.xml
    * The code generation script that built this file: zproto_server_c
    ************************************************************************

    =========================================================================
*/

//  The server runs as a background thread so that we can run multiple
//  classs at once. The API talks to the server thread over an inproc
//  pipe.

static void
s_server_task (void *args, zctx_t *ctx, void *pipe);

//  ---------------------------------------------------------------------
//  Structure of the front-end API class for rainbow_server

struct _rainbow_server_t {
    zctx_t *ctx;        //  CZMQ context
    void *pipe;         //  Pipe through to server
};


//  --------------------------------------------------------------------------
//  Create a new rainbow_server and a new server instance

rainbow_server_t *
rainbow_server_new (void)
{
    rainbow_server_t *self = (rainbow_server_t *) zmalloc (sizeof (rainbow_server_t));
    assert (self);

    //  Start a background thread for each server instance
    self->ctx = zctx_new ();
    self->pipe = zthread_fork (self->ctx, s_server_task, NULL);
    if (self->pipe) {
        char *status = zstr_recv (self->pipe);
        if (strneq (status, "OK"))
            rainbow_server_destroy (&self);
        zstr_free (&status);
    }
    else {
        free (self);
        self = NULL;
    }
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the rainbow_server and stop the server

void
rainbow_server_destroy (rainbow_server_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        rainbow_server_t *self = *self_p;
        zstr_send (self->pipe, "TERMINATE");
        char *string = zstr_recv (self->pipe);
        free (string);
        zctx_destroy (&self->ctx);
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Load server configuration data
void
rainbow_server_configure (rainbow_server_t *self, const char *config_file)
{
    zstr_sendm (self->pipe, "CONFIGURE");
    zstr_send (self->pipe, config_file);
}


//  --------------------------------------------------------------------------
//  Set one configuration key value

void
rainbow_server_setoption (rainbow_server_t *self, const char *path, const char *value)
{
    zstr_sendm (self->pipe, "SET_OPTION");
    zstr_sendm (self->pipe, path);
    zstr_send  (self->pipe, value);
}


//  --------------------------------------------------------------------------
//  Binds the server to an endpoint, formatted as printf string

long
rainbow_server_bind (rainbow_server_t *self, const char *format, ...)
{
    assert (self);
    assert (format);
    
    //  Format endpoint from provided arguments
    va_list argptr;
    va_start (argptr, format);
    char *endpoint = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Send BIND command to server task
    zstr_sendm (self->pipe, "BIND");
    zstr_send (self->pipe, endpoint);
    char *reply = zstr_recv (self->pipe);
    long reply_value = atol (reply);
    free (reply);
    free (endpoint);
    return reply_value;
}


//  ---------------------------------------------------------------------
//  State machine constants

typedef enum {
    start_state = 1,
    subscribed_state = 2,
    ready_state = 3
} state_t;

typedef enum {
    terminate_event = -1,
    subscribe_event = 1,
    publish_event = 2,
    _other_event = 3,
    have_event_event = 4
} event_t;


//  ---------------------------------------------------------------------
//  Context for the server task. This embeds the application-level
//  server context at its start, so that a pointer to server_t can
//  be cast to s_server_t for our internal use.

typedef struct {
    server_t server;            //  Application-level server context
    zctx_t *ctx;                //  Each thread has its own CZMQ context
    void *pipe;                 //  Socket to back to caller API
    void *router;               //  Socket to talk to clients
    int port;                   //  Server port bound to
    zhash_t *clients;           //  Clients we're connected to
    zconfig_t *config;          //  Configuration tree
    uint client_id;             //  Client ID counter
    size_t timeout;             //  Default client expiry timeout
    size_t monitor;             //  Server monitor interval in msec
    int64_t monitor_at;         //  Next monitor at this time
    bool terminated;            //  Server is shutting down
} s_server_t;


//  ---------------------------------------------------------------------
//  Context for each connected client. This embeds the application-level
//  client context at its start, so that a pointer to client_t can
//  be cast to s_client_t for our internal use.

typedef struct {
    client_t client;            //  Application-level client context
    char *hashkey;              //  Key into server->clients hash
    zframe_t *routing_id;       //  Routing_id back to client
    uint client_id;             //  Client ID value
    state_t state;              //  Current state
    event_t event;              //  Current event
    event_t next_event;         //  The next event
    event_t exception;          //  Exception event, if any
    bool external_state;        //  Is client in external state?
    zlist_t *requests;          //  Else, requests are queued here
    int64_t wakeup_at;          //  Wake up at this time
    event_t wakeup_event;       //  Wake up with this event
    int64_t expires_at;         //  Expires at this time
    size_t timeout;             //  Actual connection timeout
} s_client_t;

static void
    s_server_client_execute (s_server_t *server, s_client_t *client, int event);
static void
    susbcribe_the_client (client_t *self);
static void
    publish_the_event (client_t *self);

//  ---------------------------------------------------------------------
//  These methods are an internal API for actions

//  Set the next event, needed in at least one action in an internal
//  state; otherwise the state machine will wait for a message on the
//  router socket and treat that as the event.

static void
set_next_event (client_t *self, event_t event)
{
    if (self)
        ((s_client_t *) self)->next_event = event;
}

//  Raise an exception with 'event', halting any actions in progress.
//  Continues execution of actions defined for the exception event.

static void
raise_exception (client_t *self, event_t event)
{
    if (self)
        ((s_client_t *) self)->exception = (event);
}

//  Set wakeup alarm after 'delay' msecs. The next state should
//  handle the wakeup event. The alarm is cancelled on any other
//  event.

static void
set_wakeup_event (client_t *self, size_t delay, event_t event)
{
    if (self) {
        ((s_client_t *) self)->wakeup_at = zclock_time () + (delay);
        ((s_client_t *) self)->wakeup_event = (event);
    }
}

//  Execute 'event' on specified client. Use this to send events to
//  other clients. Cancels any wakeup alarm on that client.

static void
send_event (client_t *self, event_t event)
{
    if (self) {
        ((s_client_t *) self)->wakeup_at = 0;
        s_server_client_execute ((s_server_t *) self->server,
                                 (s_client_t *) self, event);
    }
}


//  Pedantic compilers don't like unused functions
static void
s_satisfy_pedantic_compilers (void)
{
    set_next_event (NULL, 0);
    raise_exception (NULL, 0);
    set_wakeup_event (NULL, 0, 0);
    send_event (NULL, 0);
}


//  ---------------------------------------------------------------------
//  Client methods

static s_client_t *
s_client_new (s_server_t *server, zframe_t *routing_id)
{
    s_client_t *self = (s_client_t *) zmalloc (sizeof (s_client_t));
    assert (self);
    assert ((s_client_t *) &self->client == self);
    
    self->state = start_state;
    self->external_state = 1;
    self->hashkey = zframe_strhex (routing_id);
    self->routing_id = zframe_dup (routing_id);
    self->requests = zlist_new ();
    self->client_id = ++(server->client_id);
    
    self->client.server = (server_t *) server;
    self->client.reply = rainbow_msg_new (0);
    rainbow_msg_set_routing_id (self->client.reply, self->routing_id);
    client_initialize (&self->client);

    return self;
}

static void
s_client_destroy (s_client_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        s_client_t *self = *self_p;
        rainbow_msg_destroy (&self->client.request);
        rainbow_msg_destroy (&self->client.reply);

        //  Clear out pending requests, if any
        rainbow_msg_t *request = (rainbow_msg_t *) zlist_first (self->requests);
        while (request) {
            rainbow_msg_destroy (&request);
            request = (rainbow_msg_t *) zlist_next (self->requests);
        }
        zlist_destroy (&self->requests);
        zframe_destroy (&self->routing_id);
        client_terminate (&self->client);
        free (self->hashkey);
        free (self);
        *self_p = NULL;
    }
}

//  Callback when we remove client from 'clients' hash table
static void
s_client_free (void *argument)
{
    s_client_t *client = (s_client_t *) argument;
    s_client_destroy (&client);
}

//  Client hash function that calculates tickless timer
static int
s_client_tickless (const char *key, void *client, void *argument)
{
    s_client_t *self = (s_client_t *) client;
    uint64_t *tickless = (uint64_t *) argument;
    if (self->expires_at
    &&  *tickless > self->expires_at)
        *tickless = self->expires_at;
    if (self->wakeup_at
    &&  *tickless > self->wakeup_at)
        *tickless = self->wakeup_at;
    return 0;
}

//  Client hash function to execute timers, if any.
//  This method might be replaced with a sorted list of timers;
//  to be tested & tuned with 10K clients; could be a utility
//  class in CZMQ

static int
s_client_timer (const char *key, void *client, void *argument)
{
    s_client_t *self = (s_client_t *) client;
    
    //  Expire client after timeout seconds of silence
    if (self->expires_at
    &&  self->expires_at <= zclock_time ()) {
        //  In case dialog doesn't handle expiry by destroying 
        //  client, cancel all timers to prevent busy-looping
        self->expires_at = 0;
        self->wakeup_at = 0;
    }
    else
    if (self->wakeup_at
    &&  self->wakeup_at <= zclock_time ()) {
        s_server_client_execute ((s_server_t *) argument, self, self->wakeup_event);
        self->wakeup_at = 0;    //  Cancel wakeup timer
    }
    return 0;
}

//  Accept request, return corresponding event

static event_t
s_client_accept (s_client_t *client, rainbow_msg_t *request)
{
    rainbow_msg_destroy (&client->client.request);
    client->client.request = request;
    switch (rainbow_msg_id (request)) {
        case RAINBOW_MSG_SUBSCRIBE:
            return subscribe_event;
            break;
        case RAINBOW_MSG_PUBLISH:
            return publish_event;
            break;
    }
    //  If we had invalid rainbow_msg_t, terminate the client
    return terminate_event;
}


//  Server methods

static void
s_server_config_self (s_server_t *self)
{
    //  Get standard server configuration
    //  Default client timeout is 60 seconds, if state machine defines
    //  an expired event; otherwise there is no timeout.
    self->timeout = atoi (
        zconfig_resolve (self->config, "server/timeout", "60")) * 1000;
    self->monitor = atoi (
        zconfig_resolve (self->config, "server/monitor", "1")) * 1000;
    self->monitor_at = zclock_time () + self->monitor;
}

static s_server_t *
s_server_new (zctx_t *ctx, void *pipe)
{
    s_server_t *self = (s_server_t *) zmalloc (sizeof (s_server_t));
    assert (self);
    assert ((s_server_t *) &self->server == self);
    server_initialize (&self->server);
    
    self->ctx = ctx;
    self->pipe = pipe;
    self->router = zsocket_new (self->ctx, ZMQ_ROUTER);
    self->clients = zhash_new ();
    self->config = zconfig_new ("root", NULL);
    s_server_config_self (self);

    s_satisfy_pedantic_compilers ();
    return self;
}

static void
s_server_destroy (s_server_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        s_server_t *self = *self_p;
        server_terminate (&self->server);
        zsocket_destroy (self->ctx, self->router);
        zconfig_destroy (&self->config);
        zhash_destroy (&self->clients);
        free (self);
        *self_p = NULL;
    }
}

//  Apply configuration tree:
//   * apply server configuration
//   * print any echo items in top-level sections
//   * apply sections that match methods

static void
s_server_apply_config (s_server_t *self)
{
    //  Apply echo commands and class methods
    zconfig_t *section = zconfig_child (self->config);
    while (section) {
        zconfig_t *entry = zconfig_child (section);
        while (entry) {
            if (streq (zconfig_name (entry), "echo"))
                zclock_log ("%s", zconfig_value (entry));
            entry = zconfig_next (entry);
        }
        if (streq (zconfig_name (section), "bind")) {
            char *endpoint = zconfig_resolve (section, "endpoint", "?");
            self->port = zsocket_bind (self->router, "%s", endpoint);
        }
        section = zconfig_next (section);
    }
    s_server_config_self (self);
}

//  Process message from pipe
static void
s_server_control_message (s_server_t *self)
{
    zmsg_t *msg = zmsg_recv (self->pipe);
    char *method = zmsg_popstr (msg);
    if (streq (method, "BIND")) {
        char *endpoint = zmsg_popstr (msg);
        self->port = zsocket_bind (self->router, "%s", endpoint);
        zstr_sendf (self->pipe, "%d", self->port);
        free (endpoint);
    }
    else
    if (streq (method, "CONFIGURE")) {
        char *config_file = zmsg_popstr (msg);
        zconfig_destroy (&self->config);
        self->config = zconfig_load (config_file);
        if (self->config)
            s_server_apply_config (self);
        else {
            printf ("E: cannot load config file '%s'\n", config_file);
            self->config = zconfig_new ("root", NULL);
        }
        free (config_file);
    }
    else
    if (streq (method, "SET_OPTION")) {
        char *path = zmsg_popstr (msg);
        char *value = zmsg_popstr (msg);
        zconfig_put (self->config, path, value);
        s_server_config_self (self);
        free (path);
        free (value);
    }
    else
    if (streq (method, "TERMINATE")) {
        zstr_send (self->pipe, "OK");
        self->terminated = true;
    }
    free (method);
    zmsg_destroy (&msg);
}


//  Execute state machine as long as we have events

static void
s_server_client_execute (s_server_t *self, s_client_t *client, int event)
{
    client->next_event = event;
    while (client->next_event) {
        client->event = client->next_event;
        client->next_event = (event_t) 0;
        client->exception = (event_t) 0;
        switch (client->state) {
            case start_state:
                if (client->event == subscribe_event) {
                    if (!client->exception) {
                        //  susbcribe the client
                        susbcribe_the_client (&client->client);
                    }
                    if (!client->exception) {
                        client->state = subscribed_state;
                        client->external_state = false;
                    }
                }
                else
                if (client->event == publish_event) {
                    if (!client->exception) {
                        //  publish the event
                        publish_the_event (&client->client);
                    }
                    if (!client->exception) {
                        client->state = ready_state;
                        client->external_state = true;
                        rainbow_msg_t *request = (rainbow_msg_t *) zlist_pop (client->requests);
                        if (request)
                            client->next_event = s_client_accept (client, request);
                    }
                }
                else {
                    //  Process all other events
                    if (!client->exception) {
                        //  send wtf
                        rainbow_msg_set_id (client->client.reply, RAINBOW_MSG_WTF);
                        rainbow_msg_send (&(client->client.reply), self->router);
                        client->client.reply = rainbow_msg_new (0);
                        rainbow_msg_set_routing_id (client->client.reply, client->routing_id);
                    }
                }
                break;

            case subscribed_state:
                if (client->event == have_event_event) {
                    if (!client->exception) {
                        //  send deliver
                        rainbow_msg_set_id (client->client.reply, RAINBOW_MSG_DELIVER);
                        rainbow_msg_send (&(client->client.reply), self->router);
                        client->client.reply = rainbow_msg_new (0);
                        rainbow_msg_set_routing_id (client->client.reply, client->routing_id);
                    }
                    if (!client->exception) {
                        client->state = subscribed_state;
                        client->external_state = false;
                    }
                }
                else {
                    //  Process all other events
                    if (!client->exception) {
                        //  send wtf
                        rainbow_msg_set_id (client->client.reply, RAINBOW_MSG_WTF);
                        rainbow_msg_send (&(client->client.reply), self->router);
                        client->client.reply = rainbow_msg_new (0);
                        rainbow_msg_set_routing_id (client->client.reply, client->routing_id);
                    }
                }
                break;

            case ready_state:
                if (client->event == publish_event) {
                    if (!client->exception) {
                        //  publish the event
                        publish_the_event (&client->client);
                    }
                }
                else {
                    //  Process all other events
                    if (!client->exception) {
                        //  send wtf
                        rainbow_msg_set_id (client->client.reply, RAINBOW_MSG_WTF);
                        rainbow_msg_send (&(client->client.reply), self->router);
                        client->client.reply = rainbow_msg_new (0);
                        rainbow_msg_set_routing_id (client->client.reply, client->routing_id);
                    }
                }
                break;

        }
        if (client->exception) {
            client->next_event = client->exception;
        }
        else {
        }
        if (client->next_event == terminate_event) {
            //  Automatically calls s_client_destroy
            zhash_delete (self->clients, client->hashkey);
            break;
        }
    }
}

//  Handle a message (a protocol request) from the client

static void
s_server_client_message (s_server_t *self)
{
    rainbow_msg_t *request = rainbow_msg_recv (self->router);
    if (!request)
        return;         //  Interrupted; do nothing

    char *hashkey = zframe_strhex (rainbow_msg_routing_id (request));
    s_client_t *client = (s_client_t *) zhash_lookup (self->clients, hashkey);
    if (client == NULL) {
        client = s_client_new (self, rainbow_msg_routing_id (request));
        zhash_insert (self->clients, hashkey, client);
        zhash_freefn (self->clients, hashkey, s_client_free);
    }
    free (hashkey);

    //  Any input from client counts as activity
    client->expires_at = zclock_time () + self->timeout;

    //  Send message to state machine if we're in an external state
    if (client->external_state)
        s_server_client_execute (self, client, s_client_accept (client, request));
    else
        //  Otherwise queue request for later
        zlist_push (client->requests, request);
}

//  Finally here's the server thread itself, which polls its two
//  sockets and processes incoming messages
static void
s_server_task (void *args, zctx_t *ctx, void *pipe)
{
    s_server_t *self = s_server_new (ctx, pipe);
    assert (self);
    zstr_send (self->pipe, "OK");

    zmq_pollitem_t items [] = {
        { self->pipe, 0, ZMQ_POLLIN, 0 },
        { self->router, 0, ZMQ_POLLIN, 0 }
    };
    self->monitor_at = zclock_time () + self->monitor;
    while (!self->terminated && !zctx_interrupted) {
        //  Calculate tickless timer, up to monitor time
        uint64_t tickless = zclock_time () + self->monitor;
        zhash_foreach (self->clients, s_client_tickless, &tickless);

        //  Poll until at most next timer event
        int rc = zmq_poll (items, 2,
            (tickless - zclock_time ()) * ZMQ_POLL_MSEC);
        if (rc == -1)
            break;              //  Context has been shut down

        //  Process incoming message from either socket
        if (items [0].revents & ZMQ_POLLIN)
            s_server_control_message (self);

        if (items [1].revents & ZMQ_POLLIN)
            s_server_client_message (self);

        //  Execute client timer events
        zhash_foreach (self->clients, s_client_timer, self);
        
        //  If clock went past timeout, then monitor server
        if (zclock_time () >= self->monitor_at) {
            self->monitor_at = zclock_time () + self->monitor;
        }
    }
    s_server_destroy (&self);
}
