#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const char *page_template =
    "<html>\n"
    "<body>\n"
    "<table cellpadding=\"4\" cellspacing=\"0\" border=\"1\">\n"
    "<thead>\n"
    "<tr>\n"
    "<th>PID</th>\n"
    "<th>Program</th>\n"
    "<th>User</th>\n"
    "<th>Group</th>\n"
    "<th>RSS&nbsp;(KB)</th>\n"
    "</tr>\n"
    "</thead>\n"
    "<tbody>\n"
    "%s"
    "</tbody>\n"
    "</table>\n"
    "</body>\n"
    "</html>\n";

static const char *error_template =
    "<html>\n"
    "<body>\n"
    "<p>Error: Could not read process information.</p>\n"
    "</body>\n"
    "</html>\n";

static int is_number(const char *str) {
  while (*str) {
    if (!isdigit(*str))
      return 0;
    str++;
  }
  return 1;
}

extern "C" void module_generate(int client_socket) {
  DIR *proc_dir;
  struct dirent *entry;
  FILE *stat_file, *status_file;
  char path[256], buffer[1024], row[512];
  char table_content[32768] = "";
  char response_buffer[65536];
  char comm[256], state;
  int pid, ppid, pgrp, session, tty_nr, tpgid;
  long utime, stime, cutime, cstime, priority, nice, num_threads, itrealvalue;
  unsigned long flags, minflt, cminflt, majflt, cmajflt, starttime, vsize, rss;
  uid_t uid;
  gid_t gid;
  struct passwd *pwd;
  struct group *grp;

  proc_dir = opendir("/proc");
  if (!proc_dir) {
    send(client_socket, error_template, strlen(error_template), 0);
    return;
  }

  // Read each entry in /proc
  while ((entry = readdir(proc_dir)) != NULL) {
    if (!is_number(entry->d_name))
      continue;

    pid = atoi(entry->d_name);

    // Read /proc/PID/stat for process information
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    stat_file = fopen(path, "r");
    if (!stat_file)
      continue;

    // Parse stat file
    if (fscanf(stat_file,
               "%d %s %c %d %d %d %d %d %lu %lu %lu %lu %lu %ld %ld %ld %ld "
               "%ld %ld %ld %ld %lu %lu %ld",
               &pid, comm, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid,
               &flags, &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime,
               &cutime, &cstime, &priority, &nice, &num_threads, &itrealvalue,
               &starttime, &vsize, &rss) != 24) {
      fclose(stat_file);
      continue;
    }
    fclose(stat_file);

    // Read /proc/PID/status for UID/GID
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    status_file = fopen(path, "r");
    uid = 0;
    gid = 0;
    if (status_file) {
      while (fgets(buffer, sizeof(buffer), status_file)) {
        if (strncmp(buffer, "Uid:", 4) == 0) {
          sscanf(buffer, "Uid:\t%d", &uid);
        } else if (strncmp(buffer, "Gid:", 4) == 0) {
          sscanf(buffer, "Gid:\t%d", &gid);
        }
      }
      fclose(status_file);
    }

    // Get username and groupname
    pwd = getpwuid(uid);
    grp = getgrgid(gid);

    // Remove parentheses from command name
    char clean_comm[256];
    if (comm[0] == '(' && comm[strlen(comm) - 1] == ')') {
      strncpy(clean_comm, comm + 1, sizeof(clean_comm) - 1);
      clean_comm[strlen(clean_comm) - 1] = '\0';
    } else {
      strcpy(clean_comm, comm);
    }

    // Table row
    snprintf(
        row, sizeof(row),
        "<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%lu</td></tr>\n",
        pid, clean_comm, pwd ? pwd->pw_name : "unknown",
        grp ? grp->gr_name : "unknown",
        rss * 4); // Convert pages to KB (assuming 4KB pages)
    if (strlen(table_content) + strlen(row) < sizeof(table_content) - 1) {
      strcat(table_content, row);
    }
  }

  closedir(proc_dir);

  snprintf(response_buffer, sizeof(response_buffer), page_template,
           table_content);

  // Send response
  send(client_socket, response_buffer, strlen(response_buffer), 0);
}
