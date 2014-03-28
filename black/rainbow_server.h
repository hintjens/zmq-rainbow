/*  =========================================================================
    rainbow_server - Rainbow server

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

#ifndef __RAINBOW_SERVER_H_INCLUDED__
#define __RAINBOW_SERVER_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _rainbow_server_t rainbow_server_t;

//  @interface
//  Create a new rainbow_server
rainbow_server_t *
    rainbow_server_new (void);

//  Destroy the rainbow_server
void
    rainbow_server_destroy (rainbow_server_t **self_p);

//  Load server configuration data
void
    rainbow_server_configure (rainbow_server_t *self, const char *config_file);

//  Set one configuration path value
void
    rainbow_server_setoption (rainbow_server_t *self, const char *path, const char *value);

//  Binds the server to an endpoint, formatted as printf string
long
    rainbow_server_bind (rainbow_server_t *self, const char *format, ...);

//  Self test of this class
void
    rainbow_server_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
