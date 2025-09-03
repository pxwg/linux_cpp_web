#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
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
    "<p>Error: Could not execute df command.</p>\n"
    "</body>\n"
    "</html>\n";

extern "C" void module_generate(int client_socket) {
  int pipe_fd[2];
  pid_t pid;
  char content_buffer[4096] = "";
  char read_buffer[1024];
  char response_buffer[8192];
  ssize_t bytes_read;
  int status;

  // Create pipe
  if (pipe(pipe_fd) == -1) {
    send(client_socket, error_template, strlen(error_template), 0);
    return;
  }

  pid = fork();
  if (pid == 0) {
    // Child process
    close(pipe_fd[0]);
    dup2(pipe_fd[1], STDOUT_FILENO);
    close(pipe_fd[1]);

    execl("/bin/df", "df", "-h", (char *)NULL);
    _exit(1);
  } else if (pid > 0) {
    // Parent process
    close(pipe_fd[1]);

    // Read from pipe into buffer
    while ((bytes_read =
                read(pipe_fd[0], read_buffer, sizeof(read_buffer) - 1)) > 0) {
      read_buffer[bytes_read] = '\0';
      strncat(content_buffer, read_buffer,
              sizeof(content_buffer) - strlen(content_buffer) - 1);
    }

    close(pipe_fd[0]);
    waitpid(pid, &status, 0); // Wait for child to finish

    // Generate HTML response using template
    snprintf(response_buffer, sizeof(response_buffer), page_template,
             content_buffer);
    send(client_socket, response_buffer, strlen(response_buffer), 0);
  } else {
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    send(client_socket, error_template, strlen(error_template), 0);
  }
}
