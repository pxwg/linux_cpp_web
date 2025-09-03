#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

static const char *page_template =
    "<html>\n"
    "<head>\n"
    "<meta http-equiv=\"refresh\" content=\"5\">\n"
    "</head>\n"
    "<body>\n"
    "<p>The current time is %s.</p>\n"
    "</body>\n"
    "</html>\n";

extern "C" void module_generate(int client_socket) {
  time_t current_time;
  char time_buffer[256];
  char response_buffer[1024];

  // Get current time
  time(&current_time);
  struct tm *time_info = localtime(&current_time);
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S %Z", time_info);

  // Generate HTML response
  snprintf(response_buffer, sizeof(response_buffer), page_template,
           time_buffer);

  // Send response
  send(client_socket, response_buffer, strlen(response_buffer), 0);
}
