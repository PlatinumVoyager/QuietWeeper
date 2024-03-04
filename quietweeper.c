#define _DEFAULT_SOURCE
/*******************************************************************************************
*
*   QUIETWEEPER v1.2.2 - GENERIC HTTP SERVER OPERATIONS
*
**********************************************************************************************/
#define INIT_QUIETWEEPER_IMPORT_LOADER      // set define directive to load quietweeping server import after

// load main server header files
#ifdef INIT_QUIETWEEPER_IMPORT_LOADER
    #define RAYGUI_IMPLEMENTATION           // let RAYGUI load import first as priority

    #ifdef RAYGUI_IMPLEMENTATION
        #include "include/raygui.h"
        #include <time.h>
        #include "raylib.h"
    #endif

    #include "include/quiet_weep.h"
    #include "include/weeping_server.h"

    #define IS_WEEPING_SERVER_ACTIVE 0x0          // default to false (use generic bit logic ! boolean)
    #define IS_WEEPING_SERVER_INACTIVE 0x1        // default to true (upon startup)

    #define MAX_PORT_LEN 0x5                      // 65535
    #define SERVER_WIDE_MAGIC_NUM 0x3
    #define GLOBAL_SERVER_INJECT_NEWLINE 0x20

    #define SERVER_BTN_INIT_SERVER_SHUTDOWN weeping_server_ui_elem[5]
    #define SERVER_BTN_INIT_SERVER_STARTUP weeping_server_ui_elem[3]

    //----------------------------------------------------------------------------------
    // QuietWeeping server function prototypes
    //----------------------------------------------------------------------------------
    static void *initialize_weeping_server(void *data);
    static void check_server_port_range(char *port);

    static void handle_server_shutdown(int signal);
    static void handle_ctrl_interrupt(int signal);

    static void weepingserver_logger(int msg_type, const char *text, va_list args); // custom server logging options

    struct stat target_stat;                                                       // directory "verity" check
    struct mg_context *global_ctx;                                                 // global mg context structure
    struct retain_udefined_opts user_defined_opts;                                 // custom operator options

    int CURRENT_SERVER_STATE = 0;                       // offline upon startup
    int IS_SERVER_RUNNING = IS_WEEPING_SERVER_ACTIVE;   // false
    int IS_SERVER_OFFLINE = IS_WEEPING_SERVER_INACTIVE; // true

    int text_input_validation_counter = 0;                      // current max text boxes filled upon startup
    int max_input_validation_counter = SERVER_WIDE_MAGIC_NUM;   // MAX amount of text input boxes to be filled

    int needs_thread = ((true - 0x0 + (true * true)) + true);   // global server context for shutdowns

    sigjmp_buf sigjmp_main;                                     // primary signal handers, application routing via local register stack/heap jmp operations (v2 revamp)
#endif


static void handle_ctrl_interrupt(int signal)
{
    handle_server_shutdown(0);

    TraceLog(LOG_WARNING, "CTRL+C SIGINT detected, shutting down the application. Server is offline...");

    exit(0);
}


int main(void)
{
    char *quiet_weeping_eye = "\t  ..,,;;;;;;,,,,,,,\n\
       .,;'';;,..,;;;,,,,,.''';;,..\n\
    ,,'' `                '';;;;,;''\n\
   ;' ```,,;@@;'  ,@@;,  @@@@;,;';.\n\
    '',;@@@@@'  ;@@@@; ''  '@@@@@;;;;\n\
     ;;@@@@@;    '''        ,@@@@@;;;\n\
    ;;@@@@@@;               ;@@@@@@;;;.\n\
     '';@@@@@,.           .,@@@@@@;;;;;;\n\
        .   '';;;;;;;;;,;;;;;' ,.:;'\n\
          ''..,,     ''''    '  .,;'\n\
               ''''''::''''''''     ,\n\
                                   ,; QUIETWEEPER\n\
                                  .;; \033[0;32m1.2.2\033[0;m\n\
                                 ,;;; Generic HTTP Server Operations\n\
                               ,;;;;:\n\
       \"Crying Eye\"         ,;@@ ;;.;\n\
                           ;;@@'  ,;\n\
                           ';;,,,;'\n\n\
===========================================================================\n";

    printf("%s\n", quiet_weeping_eye);
    printf("\n[\033[0;34m*\033[0;m] Press <ENTER> to continue...");

    getchar();

    // Set custom logger
    SetTraceLogCallback(weepingserver_logger);
    printf("\n");

    // Initialization of Window bounds, fit to max application layout spacing bounds
    //---------------------------------------------------------------------------------------
    int screenWidth = 583;
    int screenHeight = 220;

    int start_server_clicked = false;

    int server_url_link_ready = false;      // display server url yet?
    int server_static_ip_active = false;    // display server IP address
    int server_has_port_active = false;     // display server bound port to socket?

    const char *weeping_server_img_ext = ".png"; // Portable Network Graphics image extension

    // set custom icon
    Image windowIcon = LoadImageFromMemory(weeping_server_img_ext, quietweep_png, quietweep_png_len);

    InitWindow(screenWidth, screenHeight, "QUIETWEEPER v1.2.2 - GENERIC HTTP SERVER OPERATIONS");

    SetWindowIcon(windowIcon);

    //----------------------------------------------------------------------------------
    // QUIETWEEPER: controls initialization
    //----------------------------------------------------------------------------------
    const char *GroupBoxHostOptionsText = "HOST OPTIONS";                   // GROUPBOX: GroupBoxHostOptions
    const char *LabelHostnameText = "HOSTNAME:";                            // LABEL: LabelHostname
    const char *LabelPortText = "PORT:";                                    // LABEL: LabelPort
    const char *LabelPathNameText = "PATH:";                                // LABEL: LabelPathName
    const char *BtnStartHttpServerText = "START HTTP SERVER";               // BUTTON: BtnStartHttpServer

    const char *GroupBoxShutdownOperationsText = "SERVER MANAGEMENT";       // GROUPBOX: GroupBoxShutdownOperations
    const char *BtnShutdownHttpServerText = "INITIALIZE SERVER SHUTDOWN";   // BUTTON: BtnShutdownHttpServer
    
    const char *StatusBarStatusText = "INACTIVE";                           // STATUSBAR: StatusBarStatus
    const char *LabelServerModeText = "SERVER MODE:";                       // LABEL: LabelServerMode
    const char *GroupBoxStatusText = "STATUS INFORMATION";                  // GROUPBOX: GroupBoxStatus

    const char *LabelHttpBaseURLText = "HTTP BASE URL:";                    // LABEL: LabelHttpBaseURL
    const char *LabelHttpServerURLText = "NOT CONFIGURED";                  // LABEL: LabelHttpServerURL
    
    const char *LabelHttpServerIPText = "STATIC IP ADDRESS:";               // LABEL: LabelHttpServerIP
    const char *LabelHttpServerAddressText = "NOT ASSIGNED";                // LABEL: LabelHttpServerAddress

    const char *LabelBoundPortText = "BOUND PORT:";                         // LABEL: LabelBoundPort
    const char *LabelHttpServerPortText = "NO PORT BOUND";                  // LABEL: LabelHttpServerPort
    
    // Define anchors
    Vector2 anchor01 = { 248, 120 };            // ANCHOR ID:1
    Vector2 anchor02 = { 56, 16 };              // ANCHOR ID:2
    Vector2 anchor03 = { 248, 16 };             // ANCHOR ID:3
    
    // Define controls variables
    bool TextBoxHostnameEditMode = false;
    bool TextBoxPortEditMode = false;
    bool TextBoxPathNameEditMode = false;

    char TextBoxHostnameText[128] = "";            // TextBox: TextBoxHostname    
    char TextBoxPortText[128] = "";                // TextBox: TextBoxPort
    char TextBoxPathNameText[128] = "";            // TextBox: TextBoxPathName

    // LABEL UPDATE CHARACTER server_static_content_fileS
    char server_url_text[50] = "\0";         // NOT CONFIGURED || SERVER URL
    char server_static_address[16] = "\0";   // NOT ASSIGNED || SERVER IP ADDRESS
    char server_active_port[5] = "\0";       // NO PORT BOUND || PORT BOUND

    // Define controls rectangles
    Rectangle weeping_server_ui_elem[19] = {
        (Rectangle){ anchor02.x + -48, anchor02.y + 0, 224, 192 },   // GroupBox: GroupBoxHostOptions
        (Rectangle){ anchor02.x + -32, anchor02.y + 24, 144, 24 },   // Label: LabelHostname
        (Rectangle){ anchor02.x + -32, anchor02.y + 64, 168, 16 },   // Label: LabelPort
        (Rectangle){ anchor02.x + -8, anchor02.y + 144, 144, 32 },   // Button: BtnStartHttpServer
        (Rectangle){ anchor01.x + -8, anchor01.y + 24, 336, 64 },    // GroupBox: GroupBoxShutdownOperations
        (Rectangle){ 296, 160, 224, 32 },                            // Button: BtnShutdownHttpServer
        (Rectangle){ 392, 104, 88, 24 },                             // StatusBar: StatusBarStatus
        (Rectangle){ 256, 104, 96, 24 },                             // Label: LabelServerMode
        (Rectangle){ anchor03.x + -8, anchor03.y + 0, 336, 120 },    // GroupBox: GroupBoxStatus
        (Rectangle){ anchor02.x + 40, anchor02.y + 24, 120, 24 },    // TextBox: TextBoxHostname
        (Rectangle){ anchor02.x + 40, anchor02.y + 56, 120, 24 },    // TextBox: TextBoxPort
        (Rectangle){ anchor02.x + -32, anchor02.y + 96, 168, 16 },   // Label: LabelPathName
        (Rectangle){ anchor02.x + 40, anchor02.y + 88, 120, 24 },    // TextBox: TextBoxPathName
        (Rectangle){ 256, 32, 104, 16 },                             // Label: LabelHttpBaseURL
        (Rectangle){ anchor03.x + 144, anchor03.y + 16, 200, 16 },   // Label: LabelHttpServerURL
        (Rectangle){ 256, 56, 128, 16 },                             // Label: LabelHttpServerIP
        (Rectangle){ anchor03.x + 144, anchor03.y + 40, 200, 16 },   // Label: LabelHttpServerAddress
        (Rectangle){ 256, 80, 96, 16 },                              // Label: LabelBoundPort
        (Rectangle){ anchor03.x + 144, anchor03.y + 64, 200, 16 },   // Label: LabelHttpServerPort
    };
    //----------------------------------------------------------------------------------

    SetTargetFPS(120);

    // custom styling
    const char *RGS_TARGET = "cyber.rgs";
    GuiLoadStyle(RGS_TARGET);

    // main quietweeper server thread
    pthread_t server_thread;

    // obtain proper lengths to display label text correctly aligned
    size_t not_configured_len = snprintf(NULL, 0, "%s", LabelHttpServerURLText);
    size_t not_assigned_len = snprintf(NULL, 0, "%s", LabelHttpServerAddressText);
    size_t no_port_bound_len = snprintf(NULL, 0, "%s", LabelHttpServerPortText);

    size_t server_base_url_len = snprintf(NULL, 0, "http://%s:%s", user_defined_opts.target_hostname, user_defined_opts.target_port);
    size_t server_static_address_len = snprintf(NULL, 0, "%s", user_defined_opts.target_hostname);

    //--------------------------------------------------------------------------------------
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // set a SIGINT signal interrupt handler
        if (signal(SIGINT, handle_ctrl_interrupt) == ((__sighandler_t) -1))
        {
            TraceLog(LOG_ERROR, "failed to setup a signal interrupt handler (SIGINT). Terminate QuietWeeper by clicking out of the actual GUI not CLI. :(");
        }
    
        //----------------------------------------------------------------------------------
        // MAIN APPLICATION UPDATE LOGIC
        //----------------------------------------------------------------------------------

        // update HTTP base URL
        if (server_url_link_ready)
        {
            snprintf(server_url_text, (server_base_url_len * 0x2), "http://%s:%s", user_defined_opts.target_hostname, user_defined_opts.target_port);
            GuiLabel(weeping_server_ui_elem[14], server_url_text);
        }
        else 
        {
            snprintf(server_url_text, (not_configured_len + 1), "%s", LabelHttpServerURLText);
            GuiLabel(weeping_server_ui_elem[14], server_url_text);
        }

        // update server IP address
        if (server_static_ip_active)
        {
            if (max_input_validation_counter == SERVER_WIDE_MAGIC_NUM)
            {
                snprintf(server_static_address, (server_static_address_len * (SERVER_WIDE_MAGIC_NUM - 1)), "%s", user_defined_opts.target_hostname);
                GuiLabel(weeping_server_ui_elem[16], server_static_address);
            }
        }
        else 
        {
            snprintf(server_static_address, (not_assigned_len + 1), "%s", LabelHttpServerAddressText);
            GuiLabel(weeping_server_ui_elem[16], server_static_address);
        }

        // update server port
        if (server_has_port_active)
        {
            snprintf(server_active_port, (MAX_PORT_LEN + 1), "%s", user_defined_opts.target_port);
            GuiLabel(weeping_server_ui_elem[18], server_active_port);
        }
        else
        {
            snprintf(server_active_port, (no_port_bound_len + 1), "%s", LabelHttpServerPortText);
            GuiLabel(weeping_server_ui_elem[18], server_active_port);
        }

        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 

        //----------------------------------------------------------------------------------
        // Draw controls
        //----------------------------------------------------------------------------------
        GuiGroupBox(weeping_server_ui_elem[0], GroupBoxHostOptionsText);

        GuiLabel(weeping_server_ui_elem[1], LabelHostnameText);
        GuiLabel(weeping_server_ui_elem[2], LabelPortText);

        GuiSetStyle(STATUSBAR, BACKGROUND_COLOR, 0x000000FF);

        // start the HTTP/1.1 server
        if (GuiButton(SERVER_BTN_INIT_SERVER_STARTUP, BtnStartHttpServerText) || (IS_SERVER_RUNNING == false && IsKeyPressed(KEY_ENTER)))
        {
            if (IS_SERVER_RUNNING)
            {
                TraceLog(LOG_WARNING, "The server is already running!");

                start_server_clicked = false;
            }
            else if (!IS_SERVER_RUNNING && !start_server_clicked)
            {
                size_t host_length = snprintf(NULL, 0, "%s", TextBoxHostnameText);
                size_t port_length = snprintf(NULL, 0, "%s", TextBoxPortText);
                size_t path_length = snprintf(NULL, 0, "%s", TextBoxPathNameText);

                switch (host_length)
                { 
                    case 0x0: 
                    { 
                        TraceLog(LOG_ERROR, "A proper TCP/IP capable hostname is required to be defined!"); 

                        break;
                    } 
                    
                    // generic counter for user input "validation"
                    default:
                    {
                        // check hostname (make GLOBAL)
                        user_defined_opts.target_hostname = strdup(TextBoxHostnameText);

                        text_input_validation_counter++;
                    }
                }

                switch (port_length)
                { 
                    case 0x0: 
                    { 
                        TraceLog(LOG_ERROR, "A non-zero port is required to be defined!"); 

                        break;
                    }

                    default:
                    {
                        // call function to check port range
                        check_server_port_range(TextBoxPortText);
                    }
                }

                switch (path_length)
                {
                    case 0x0: 
                    {
                        TraceLog(LOG_ERROR, "An absolute path to the document root is required!");

                        break;
                    }

                    default: 
                    {
                        // check for valid path
                        check_udefined_path(TextBoxPathNameText);
                    }
                }

                if (text_input_validation_counter == max_input_validation_counter)
                {
                    text_input_validation_counter = 0x0;

                    StatusBarStatusText = "ACTIVE";

                    // get server ip address
                    user_defined_opts.target_path = strdup(TextBoxPathNameText);
                    user_defined_opts.target_port = strdup(TextBoxPortText); // get server port address
                    
                    TraceLog(LOG_INFO, "Set server bind/bound port to %s", OPERATOR_PORTLET);

                    user_defined_opts.set_value = (SERVER_WIDE_MAGIC_NUM - 0x1);

                    TraceLog(LOG_INFO, "DIRECTORY ROOT VERITY CHECK (inode #): \033[0;32m%ld\033[0;m", target_stat.st_ino);

                    // start HTTP server
                    if (pthread_create(&server_thread, NULL, initialize_weeping_server, &needs_thread) != 0x0)
                    {
                        TraceLog(LOG_ERROR, "failed to create main server thread!\n");

                        exit(-1);
                    }

                    IS_SERVER_RUNNING = true;

                    start_server_clicked = true;    // operator clicked start server

                    server_url_link_ready = true;   // HTTP BASE URL
                    server_static_ip_active = true; // SERVER IP ADDRESS
                    server_has_port_active = true;  // BOUND PORT
                    
                    if (IS_SERVER_RUNNING == true)
                    {
                        GuiSetStyle(STATUSBAR, TEXT_COLOR_NORMAL, 0x00FF00FF); // RGB + ALPHA
                        CURRENT_SERVER_STATE = (IS_WEEPING_SERVER_ACTIVE + true);
                    }
                    else
                    {
                        TraceLog(LOG_ERROR, "Server called from init server function, yet did not initialize the server...");

                        exit(0);
                    }

                    TraceLog(LOG_INFO, "HTTP Server started: %s:%s", TextBoxHostnameText, TextBoxPortText);
                }
                else
                {
                    TraceLog(LOG_ERROR, "You must produce valid input for ALL options before proceeding.");
                
                    text_input_validation_counter = 0x0;
                }
            }
        }
        
        GuiGroupBox(weeping_server_ui_elem[4], GroupBoxShutdownOperationsText);

        // shutdown HTTP server
        if (GuiButton(SERVER_BTN_INIT_SERVER_SHUTDOWN, BtnShutdownHttpServerText) || (IS_SERVER_RUNNING == true && IsKeyPressed(KEY_D) && user_defined_opts.set_value == SERVER_WIDE_MAGIC_NUM))
        {
            // only possible to shutdown if the server is already running
            if (IS_SERVER_RUNNING)
            {
                StatusBarStatusText = "INACTIVE";

                TraceLog(LOG_INFO, "Server shutting down...");

                needs_thread = false;

                // shutdown server
                handle_server_shutdown(0);

                IS_SERVER_RUNNING = false;
                IS_SERVER_OFFLINE = (IS_WEEPING_SERVER_INACTIVE * 0x2); // dos

                pthread_join(server_thread, NULL);
        
                CURRENT_SERVER_STATE = IS_WEEPING_SERVER_INACTIVE; // true

                if (IS_SERVER_OFFLINE == (IS_WEEPING_SERVER_INACTIVE + IS_WEEPING_SERVER_ACTIVE + 1))
                {
                    GuiSetStyle(STATUSBAR, TEXT_COLOR_NORMAL, 0xFF0000FF);
                    
                    // reset text globally until server init
                    server_url_link_ready = false;
                    server_static_ip_active = false;
                    server_has_port_active = false;

                    start_server_clicked = false; // server is offline
                }
                else 
                {
                    TraceLog(LOG_ERROR, "The server seems to be down, yet it appears it is not conveying that to you..the end user :(");

                    exit(0);
                }

                text_input_validation_counter = 0;
                
                free(user_defined_opts.target_path);
                free(user_defined_opts.target_port);
            }
            else
                if (GuiButton(weeping_server_ui_elem[5], BtnShutdownHttpServerText) || (IS_SERVER_RUNNING == false && IsKeyPressed(KEY_D)))
                    TraceLog(LOG_WARNING, "The server is not running, not attempting a shutdown.");
        } 

        GuiStatusBar(weeping_server_ui_elem[6], StatusBarStatusText);
        GuiLabel(weeping_server_ui_elem[7], LabelServerModeText);
        GuiGroupBox(weeping_server_ui_elem[8], GroupBoxStatusText);

        if (GuiTextBox(weeping_server_ui_elem[9], TextBoxHostnameText, 128, TextBoxHostnameEditMode)) TextBoxHostnameEditMode = !TextBoxHostnameEditMode;
        if (GuiTextBox(weeping_server_ui_elem[10], TextBoxPortText, 128, TextBoxPortEditMode)) TextBoxPortEditMode = !TextBoxPortEditMode;
        
        GuiLabel(weeping_server_ui_elem[11], LabelPathNameText);
        
        if (GuiTextBox(weeping_server_ui_elem[12], TextBoxPathNameText, 128, TextBoxPathNameEditMode)) TextBoxPathNameEditMode = !TextBoxPathNameEditMode;
        
        GuiLabel(weeping_server_ui_elem[13], LabelHttpBaseURLText);  // HTTP BASE URL
        GuiLabel(weeping_server_ui_elem[15], LabelHttpServerIPText); // HTTP STATIC IP
        GuiLabel(weeping_server_ui_elem[17], LabelBoundPortText);    // HTTP BOUND PORT
       
        //----------------------------------------------------------------------------------
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


static void check_server_hostname(void)
{
    // utilize global variable
    return;
}


static void check_server_port_range(char *port)
{
    int target_port = (int) atoi(port);
    size_t path_len = snprintf(NULL, 0, "%s", user_defined_opts.target_path);

    if (target_port < 1023)
    {
        uid_t uid = getuid();

        // check for root priv
        if (uid == 0)
            text_input_validation_counter++;
        else
            TraceLog(LOG_WARNING, "You must be root (uid=0) in order to use reserved system ports, unless probed for an exploit otherwise..run this with sudo.");
    }
    else if (target_port > 65535)
    {
        TraceLog(LOG_ERROR, "Impossible set-port operation detected, port range out of bounds!");

        return;
    }
    else 
    {
        if (IS_SERVER_RUNNING == false && path_len != 0x0)
        {
            text_input_validation_counter++;
        }
    }

    return;
}


static void *initialize_weeping_server(void *data)
{
    if (__sigsetjmp(sigjmp_main, 0x1) == 0x0)
    {
        if (signal(0x2, handle_server_shutdown) == ((__sighandler_t) -1))
        {
            TraceLog(LOG_ERROR, "Failed to setup a proper SIGINT handler!");

            exit(1);
        }
    }

    int *needs_thread = (int *)data;

    struct mg_callbacks callbacks;

    // start civetweb callbacks
    mg_init_library(0);

    // zero out structure
    memset(&callbacks, 0x0, sizeof(callbacks));

    // called upon new http request to the server
    callbacks.begin_request = begin_request_handler;

    size_t final_sz = snprintf(NULL, 0, "%s:%s", user_defined_opts.target_hostname, user_defined_opts.target_port);

    char *final_host = (char *) malloc(sizeof(final_host) * final_sz);

    snprintf(final_host, (final_sz + 0x1), "%s:%s", user_defined_opts.target_hostname, user_defined_opts.target_port);

    // define server wide options
    const char *server_opts[] = {
        "document_root", user_defined_opts.target_path,     // document root
        "listening_ports", final_host,                      // server instance port bound to local socket
        "index_files", "index.html",                        // change source if you do not like "index.html" as the default opt
        NULL                                                // END OF OPTIONS
    };

    user_defined_opts.set_value += 0x1;

    // start the web server
    global_ctx = mg_start(&callbacks, &user_defined_opts, server_opts);

    TraceLog(LOG_INFO, "Starting server on port %s", mg_get_option(global_ctx, "listening_ports"));
    TraceLog(LOG_INFO, "Document Root: %s", mg_get_option(global_ctx, "document_root"));

    // run threaded server until "initialize server shutdown" button is clicked
    do { ;; } while (*needs_thread == 1);

    return NULL;
}


static void handle_server_shutdown(int signal)
{
    goto MG_EXIT_LIBRARY_;

    MG_EXIT_LIBRARY_:
    {
        INITIALIZE_SERVER_SHUTDOWN

        TraceLog(LOG_INFO, "Server successfully shutdown");
    }

    return;
}


// bundle it up baby
static int begin_request_handler(struct mg_connection *conn)
{
    struct retain_udefined_opts *user_opts = (struct retain_udefined_opts *)mg_get_user_data(global_ctx);
    const struct mg_request_info *request_info = mg_get_request_info(conn);

    TraceLog
    (
        LOG_INFO, 
        "ENDPOINT CLIENT: \033[0;32m%s:%d\033[0;m >> HTTP/%s %s request for URI %s", 
            request_info->remote_addr,
            request_info->remote_port,
            request_info->http_version,
            request_info->request_method,
            request_info->request_uri
    );

    if (strcmp(request_info->request_method, "GET") == 0x0 && strcmp(request_info->local_uri, "/") == 0x0)
    {
        char file[1024];
        snprintf(file, sizeof(file), "%s%s/index.html", user_opts->target_path, request_info->local_uri);

		// did the user suppliment a path? if so add it here, otherwise host current directory
        serve_static_file(conn, file, "text/html");
    }
    else 
    {
        char path[1024];
        snprintf(path, sizeof(path), "%s%s", user_opts->target_path, request_info->local_uri);

        serve_static_file(conn, path, get_content_type(request_info->local_uri, request_info->local_uri));
    }

    return 1;
}


// determine from local uniform resource identifier HTTP/1.1
static const char *get_content_type(const char *target_path, const char *local_uri)
{
   	const char *ext = strrchr(target_path, '.');

    // client ip address
	
	if (ext == NULL)
	{
		TraceLog(LOG_ERROR, "get_content_type() => ext = NULL!");

		exit(0);
	}
	else
	{
		if (strcmp(ext, ".html") == 0x0)
		{
            TraceLog(LOG_INFO, "Returning type text/html for resource \"%s\" of type .html to client", local_uri);

			// static HTML file
			return "text/html";
		}
		else if (strcmp(ext, ".css") == 0x0)
		{
            TraceLog(LOG_INFO, "Returning type text/css for resource \"%s\" of type .css to client", local_uri);

			// static CSS file
			return "text/css";
		}
		else if (strcmp(ext, ".js") == 0x0)
		{
            TraceLog(LOG_INFO, "Returning type application/javascript for resource \"%s\" of type .js to client", local_uri);

			// static JAVASCRIPT file
			return "application/javascript";
		}
		else if (strcmp(ext, ".jpg") == 0x0)
		{
            TraceLog(LOG_INFO, "Returning type image/jpeg (Joint Photographics Experts Group) for resource \"%s\" of type .jpeg to client", local_uri);

			// static JPEG (Joint Photographic Experts Group) file
			return "image/jpeg";
		}
		else if (strcmp(ext, ".png") == 0x0)
		{
            TraceLog(LOG_INFO, "Returning type image/png (Portable Network Graphics) for resource \"%s\" of type .png to client", local_uri);

			// static PNG (Portable Network Graphics) file
			return "image/png";
		}
	}

	return "text/plain";
}


static void check_udefined_path(char *target)
{
    // obtain information about the file
    if (stat(target, &target_stat) == 0x0)
    {
        text_input_validation_counter++;
    }
    else
    {
        // also pass error to QUIETWEEPER console
        TraceLog(LOG_ERROR, "The defined path \"%s\" does not exist.", target);
    }

    return;
}


static void serve_static_file(struct mg_connection *conn, const char *file, const char *content_type)
{
    // static file to server
    FILE *fp;

    long length;
    char *server_static_content_file;

    fp = fopen(file, "rb");

    if (fp == NULL)
    {
        // failed to open the file
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n");
        
		// check for null file
		if (strlen(file) < 0x1)
		{
			mg_printf(conn, "Resource request error: You failed :(\r\n");

            return;
		}
		else
		{
			mg_printf(conn, "Resource request error: %s was not found within the bounds of the server.\r\n", file);
		
			return;
		}

        return;
    }

    // file exists, obtain the length
    fseek(fp, 0, SEEK_END);     // to the end
    length = ftell(fp);         // what is our current position in the byte stream?
    fseek(fp, 0, SEEK_SET);     // now back to the top

    // allocate memory for the file content
    server_static_content_file = (char *) malloc(sizeof(char) * length);

    if (server_static_content_file == NULL)
    {
        fclose(fp);

        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n");
        mg_printf(conn, "500 Internal Server Error: failed to allocate memory\r\n");

        return;
    }

    // transfer file content into the server_static_content_file
    fread(server_static_content_file, 0x1, length, fp);
    fclose(fp);

    printf("\n");

    TraceLog(LOG_INFO, "Printing static file mapping below:\n%s\n", server_static_content_file);
    TraceLog(LOG_INFO, "File: %s\n\tFile size: \033[0;32m%lu\033[0;32m\033[0;m bytes", file, length);

    // save the content as the http response
    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", content_type, length);
    mg_send_mime_file(conn, file, content_type);

    free(server_static_content_file);

    return;
}


// Custom logging function
static void weepingserver_logger(int msg_type, const char *text, va_list args)
{
    char timeStr[64] = { 0x0 };
    time_t now = time(NULL);
    
    struct tm *tm_info = localtime(&now);

    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("QUIETWEEPER ");

    switch (msg_type)
    {
        // 0x3
        case LOG_INFO:
        {
            printf("\033[0;32m[INFO]\033[0;m [%s] ", timeStr); 
            
            break;
        }

        // 0x5
        case LOG_ERROR:
        {
            printf("\033[0;31m[ERROR]\033[0;m [%s] ", timeStr);

            break;
        }

        // 0x4
        case LOG_WARNING:
        {
            printf("\033[0;33m[WARN]\033[0;m [%s] ", timeStr); 
            
            break;
        }

        default: 
            break;
    }

    vprintf(text, args);
    printf("\n");
}

// 2024 QUIETWEEPING HTTP SERVER AUTHORS