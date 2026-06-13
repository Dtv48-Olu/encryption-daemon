# Native Encryption Daemon

> **Status: ⚠️ Actively in Development** — This project is a work in progress and subject to breaking changes.

A high-performance Linux daemon that monitors user-specified directories and automatically encrypts files using authenticated encryption via libsodium, leveraging Go concurrency primitives and a native C++ cryptography engine.

## Tech Stack

- **Runtime**: Go 1.21+
- **Concurrency**: Goroutines, channels
- **Encryption Engine**: C++17, libsodium
- **IPC**: Subprocess communication, pipes
- **File Monitoring**: Linux inotify
- **Platform**: Fedora Linux
- **Build Tools**: CMake, Make, GCC/Clang

## Getting Started

### Prerequisites

- Go 1.21 or later
- C++ compiler (g++ or clang++)
- libsodium development libraries (`libsodium-devel` on Fedora)
- CMake 3.16 or later
- Linux kernel with inotify support

### Building

```bash
# Clone the repository
git clone https://github.com/Dtv48-olu/encryption-daemon.git
cd encryption-daemon

# Build everything currently implemented (C++ engine + Go daemon)
make

# Install the daemon (optional)
sudo make install
```

### Running the Daemon

```bash
# Start as a systemd service (post-installation)
sudo systemctl start encryption-daemon
sudo systemctl enable encryption-daemon

# Or run directly against one or more watched directories
./encryption-daemon -engine ./build/crypto/encryption-engine ~/Documents/sensitive

# View daemon logs
journalctl -u encryption-daemon -f
```

### Using the CLI Tool

```bash
# Watch a directory (auto-encrypts new files)
encryption-ctl watch ~/Documents/sensitive

# Stop watching a directory
encryption-ctl unwatch ~/Documents/sensitive

# Encrypt a directory immediately
encryption-ctl encrypt-now ~/Documents/sensitive

# Check daemon status
encryption-ctl status

# Stop the daemon
encryption-ctl stop
```

## Key Features (Roadmap)

- [ ] Real-time directory monitoring with inotify
- [ ] Concurrent file processing with goroutines
- [ ] Authenticated file encryption via libsodium
- [ ] Dynamic directory management via CLI tool
- [ ] Persistent watch state across restarts
- [ ] Systemd service integration
- [ ] Secure key and memory management

## Architecture

For a full breakdown of design decisions, components, data flow, and the communication model see [ARCHITECTURE.md](./ARCHITECTURE.md).

## License

MIT License
