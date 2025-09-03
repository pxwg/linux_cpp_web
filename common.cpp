#include "server.h"
#include <iostream>
#include <string>

// Global variables
// TODO: Better management of http templates and global settings
// (e.g., in a config struct)
int verbose_mode = 0;
const char *module_dir = "./";

// HTTP response templates
const char *ok_response = "HTTP/1.0 200 OK\n"
                          "Content-type: text/html\n"
                          "\n";

const char *bad_request_response =
    "HTTP/1.0 400 Bad Request\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    "<body>\n"
    "<h1>Bad Request</h1>\n"
    "<p>This server did not understand your request.</p>\n"
    "</body>\n"
    "</html>\n";

const char *not_found_response_template =
    "HTTP/1.0 404 Not Found\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    "<body>\n"
    "<h1>Not Found</h1>\n"
    "<p>The requested URL %s was not found on this server.</p>\n"
    "</body>\n"
    "</html>\n";

const char *bad_method_response_template =
    "HTTP/1.0 501 Method Not Implemented\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    "<body>\n"
    "<h1>Method Not Implemented</h1>\n"
    "<p>The method %s is not implemented by this server.</p>\n"
    "</body>\n"
    "</html>\n";

// Utility function to log verbose messages
void log_verbose(const char *message) {
  if (verbose_mode) {
    std::cout << "[VERBOSE] " << message << std::endl;
  }
}
