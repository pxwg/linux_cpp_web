#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const char *page_template = "<html>\n"
                                   "<body>\n"
                                   "<pre>\n"
                                   "%s"
                                   "</pre>\n"
                                   "</body>\n"
                                   "</html>\n";

static const char *error_template =
    "<html>\n"
    "<body>\n"
    "<p>Error: Could not read system issue information.</p>\n"
    "</body>\n"
    "</html>\n";

extern "C" void module_generate(int client_socket) {
  FILE *issue_file;
  char content_buffer[4096] = "";
  char line_buffer[256];
  char response_buffer[8192];

  // Try to open /proc/issue
  issue_file = fopen("/proc/issue", "r");
  if (!issue_file) {
    // If can't open /proc/issue, try /etc/issue as fallback
    issue_file = fopen("/etc/issue", "r");
  }

  if (issue_file) {
    while (fgets(line_buffer, sizeof(line_buffer), issue_file)) {
      strncat(content_buffer, line_buffer,
              sizeof(content_buffer) - strlen(content_buffer) - 1);
    }
    fclose(issue_file);

    // Generate HTML response using template
    snprintf(response_buffer, sizeof(response_buffer), page_template,
             content_buffer);
  } else {
    // Use error template
    strcpy(response_buffer, error_template);
  }

  // Send response
  send(client_socket, response_buffer, strlen(response_buffer), 0);
}
