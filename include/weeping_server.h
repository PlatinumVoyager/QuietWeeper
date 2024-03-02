#ifdef INIT_QUIETWEEPER_IMPORT_LOADER
    // main server thread logic 
    #include <pthread.h>

    // primary web server functionality header file 
	#include <civetweb.h>

    // user defined options, "take 0ver" default raygui/raylib
    #include <getopt.h>

    // signal and jmp operations
    #include <signal.h>
    #include <setjmp.h>

    // file specific functions
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <ctype.h>

    #include <stdlib.h>

    #include <unistd.h>
	#include <arpa/inet.h>
#endif

#define AND &&
#define OR ||
#define NOT !

#define OPERATOR_PATHLET user_defined_opts.target_path
#define OPERATOR_PORTLET user_defined_opts.target_port

#define INITIALIZE_SERVER_SHUTDOWN \
        mg_stop(global_ctx); \
        mg_exit_library(); \

// static void *initialize_weeping_server(void *data);
static void handle_server_shutdown(int signal);

static void check_udefined_path(char *target);
static void check_server_hostname(void);

static int begin_request_handler(struct mg_connection *conn);
static const char *get_content_type(const char *target_path, const char *local_uri);
static void serve_static_file(struct mg_connection *conn, const char *file, const char *content_type);

// custom user related data structure
struct retain_udefined_opts
{
    // is custom path set?
    int set_value;

    // custom interface to bind to 
    char *target_hostname;

    // custom port to bind to socket
    char *target_port;

    // custom path to host server
    char *target_path;
};

// 2024 QUIETWEEPING HTTP SERVER AUTHORS