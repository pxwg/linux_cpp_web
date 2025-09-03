# Linux Cpp Web Server

## Quick Start

```bash
# Build the project
make

# Start server on localhost:8008
./server --address 127.0.0.1 --port 8008 --verbose

# Test the server
curl http://127.0.0.1:8008/
curl http://127.0.0.1:8008/time
curl http://127.0.0.1:8008/diskfree
curl http://127.0.0.1:8008/process
```

## Command Line Options

```
Usage: ./server [OPTIONS]
Options:
  -a, --address ADDRESS   Server address (default: localhost)
  -p, --port PORT         Server port (default: 8008)
  -m, --moddir DIR        Module directory (default: ./)
  -v, --verbose           Verbose mode
  -h, --help              Show help message
```

## Available Modules

> Unfinished for now

- `/time` - Display current system time (auto-refresh)
- `/issue` - Display system release information
- `/diskfree` - Display disk usage statistics
- `/process` - Display running processes table
