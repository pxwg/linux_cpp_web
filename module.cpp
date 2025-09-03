#include "server.h"
#include <iostream>
#include <string>

// External utility function
extern void log_verbose(const char *message);

struct server_module *module_open(const char *module_name) {
  struct server_module *module = nullptr;
  std::string full_path;

  // Allocate memory for module structure
  module = (struct server_module *)malloc(sizeof(struct server_module));
  if (!module) {
    std::cerr << "Failed to allocate memory for module" << std::endl;
    return nullptr;
  }

  // Construct full path to module
  full_path = std::string(module_dir) + "/" + module_name + ".so";

  if (verbose_mode) {
    std::cout << "[VERBOSE] Loading module: " << full_path << std::endl;
  }

  // Load the dynamic library
  module->handle = dlopen(full_path.c_str(), RTLD_LAZY);
  if (!module->handle) {
    std::cerr << "Cannot load module " << full_path << ": " << dlerror()
              << std::endl;
    free(module);
    return nullptr;
  }

  // Clear any existing error
  dlerror();

  // Get the module_generate function
  module->generate_function =
      (void (*)(int))dlsym(module->handle, "module_generate");

  const char *dlsym_error = dlerror();
  if (dlsym_error) {
    std::cerr << "Cannot load symbol 'module_generate': " << dlsym_error
              << std::endl;
    dlclose(module->handle);
    free(module);
    return nullptr;
  }

  // Store module name
  module->name = strdup(module_name);

  if (verbose_mode) {
    std::cout << "[VERBOSE] Successfully loaded module: " << module_name
              << std::endl;
  }

  return module;
}

void module_close(struct server_module *module) {
  if (!module)
    return;

  if (verbose_mode) {
    std::cout << "[VERBOSE] Closing module: " << module->name << std::endl;
  }

  // Close the dynamic library
  if (module->handle) {
    dlclose(module->handle);
  }

  // Free the module name
  if (module->name) {
    free((void *)module->name);
  }

  // Free the module structure
  free(module);
}
