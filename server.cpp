#include "server.h"
#include <iostream>
#include <signal.h>
#include <sys/wait.h>

// External utility function
extern void log_verbose(const char *message);

void handle_request(int client_socket) {
  char buffer[1024];
  char method[16], url[256], protocol[16];

  // Read the request
  ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
  if (bytes_read <= 0) {
    close(client_socket);
    return;
  }

  buffer[bytes_read] = '\0';

  if (verbose_mode) {
    std::cout << "[VERBOSE] Received request:\n" << buffer << std::endl;
  }

  // Parse the HTTP request
  if (sscanf(buffer, "%15s %255s %15s", method, url, protocol) != 3) {
    send(client_socket, bad_request_response, strlen(bad_request_response), 0);
    close(client_socket);
    return;
  }

  // Only handle GET requests
  if (strcmp(method, "GET") != 0) {
    char response[1024];
    snprintf(response, sizeof(response), bad_method_response_template, method);
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
    return;
  }

  // Extract module name from URL (remove leading '/')
  const char *module_name = url + 1;

  // Handle root request
  if (strlen(module_name) == 0 || strcmp(module_name, "/") == 0) {
    const char *index_page =
        "HTTP/1.0 200 OK\n"
        "Content-type: text/html\n"
        "\n"
        "<html>\n"
        "<body>\n"
        "<h1>System Information Server</h1>\n"
        "<p>Available modules:</p>\n"
        "<ul>\n"
        "<li><a href=\"/time\">System Time</a></li>\n"
        "<li><a href=\"/issue\">System Information</a></li>\n"
        "<li><a href=\"/diskfree\">Disk Space</a></li>\n"
        "<li><a href=\"/process\">Running Processes</a></li>\n"
        "</ul>\n"
        "</body>\n"
        "</html>\n";

    send(client_socket, index_page, strlen(index_page), 0);
    close(client_socket);
    return;
  }

  // Try to load and execute the requested module
  struct server_module *module = module_open(module_name);
  if (!module) {
    char response[1024];
    snprintf(response, sizeof(response), not_found_response_template, url);
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
    return;
  }

  // Send OK response header
  send(client_socket, ok_response, strlen(ok_response), 0);

  // Execute the module function
  module->generate_function(client_socket);

  // Clean up
  module_close(module);
  close(client_socket);
}

void server_run(struct in_addr local_address, uint16_t port) {
  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    perror("socket creation failed");
    exit(1);
  }

  // Set socket options
  int reuse = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse,
                 sizeof(reuse)) < 0) {
    perror("setsockopt failed");
    close(server_socket);
    exit(1);
  }

  // Configure server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr = local_address;
  server_addr.sin_port = htons(port);

  // Bind socket
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("bind failed");
    close(server_socket);
    exit(1);
  }

  // Listen for connections
  if (listen(server_socket, 10) < 0) {
    perror("listen failed");
    close(server_socket);
    exit(1);
  }

  std::cout << "Server listening on " << inet_ntoa(local_address) << ":" << port
            << std::endl;

  // Ignore SIGCHLD to prevent zombie processes
  signal(SIGCHLD, SIG_IGN);

  // Main server loop
  while (true) {
    client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
      if (errno == EINTR)
        continue; // Interrupted system call
      perror("accept failed");
      continue;
    }

    if (verbose_mode) {
      std::cout << "[VERBOSE] Connection from "
                << inet_ntoa(client_addr.sin_addr) << std::endl;
    }

    // Handle request in child process
    pid_t pid = fork();
    if (pid == 0) {
      // Child process
      close(server_socket);
      handle_request(client_socket);
      exit(0);
    } else if (pid > 0) {
      // Parent process
      close(client_socket);
    } else {
      perror("fork failed");
      close(client_socket);
    }
  }

  close(server_socket);
}
