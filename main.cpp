#include "server.h"
#include <getopt.h>
#include <iostream>

extern int verbose_mode;
extern const char *module_dir;

void print_usage(const char *program_name) {
  std::cout << "Usage: " << program_name << " [OPTIONS]\n"
            << "Options:\n"
            << "  -a, --address ADDRESS   Server address (default: localhost)\n"
            << "  -p, --port PORT         Server port (default: 8008)\n"
            << "  -m, --moddir DIR        Module directory (default: ./)\n"
            << "  -v, --verbose           Verbose mode\n"
            << "  -h, --help              Show this help message\n"
            << std::endl;
}

int main(int argc, char *argv[]) {
  const char *address_str = "127.0.0.1";
  uint16_t port = 8008;
  struct in_addr local_address;

  // Define long options
  static struct option long_options[] = {{"address", required_argument, 0, 'a'},
                                         {"help", no_argument, 0, 'h'},
                                         {"moddir", required_argument, 0, 'm'},
                                         {"port", required_argument, 0, 'p'},
                                         {"verbose", no_argument, 0, 'v'},
                                         {0, 0, 0, 0}};

  // Parse command line arguments
  int opt;
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "a:hm:p:v", long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'a':
      address_str = optarg;
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    case 'm':
      module_dir = optarg;
      break;
    case 'p':
      port = (uint16_t)atoi(optarg);
      if (port == 0) {
        std::cerr << "Invalid port number: " << optarg << std::endl;
        return 1;
      }
      break;
    case 'v':
      verbose_mode = 1;
      break;
    case '?':
      print_usage(argv[0]);
      return 1;
    default:
      break;
    }
  }

  // Convert address string to in_addr
  if (inet_aton(address_str, &local_address) == 0) {
    std::cerr << "Invalid address: " << address_str << std::endl;
    return 1;
  }

  if (verbose_mode) {
    std::cout << "[VERBOSE] Starting server on " << address_str << ":" << port
              << std::endl;
    std::cout << "[VERBOSE] Module directory: " << module_dir << std::endl;
  }

  // Start the server
  server_run(local_address, port);

  return 0;
}
