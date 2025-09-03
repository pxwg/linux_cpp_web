#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Module structure for dynamic loading
struct server_module {
  void *handle;
  const char *name;
  void (*generate_function)(int);
};

// Function declarations for module management
extern struct server_module *module_open(const char *module_name);
extern void module_close(struct server_module *module);

// Function declaration for server operation
extern void server_run(struct in_addr local_address, uint16_t port);

// Global variables
extern int verbose_mode;
extern const char *module_dir;

// HTTP response templates
extern const char *ok_response;
extern const char *bad_request_response;
extern const char *not_found_response_template;
extern const char *bad_method_response_template;

// Utility functions
extern void log_verbose(const char *message);

#endif // SERVER_H
