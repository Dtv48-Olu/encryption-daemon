# Native Encryption Daemon

> **Status: ⚠️ Actively in Development** — This project is a work in progress and subject to breaking changes.

A high-performance Linux daemon that monitors specified directories and automatically encrypts files using AES-256 encryption, leveraging Go concurrency primitives and a native C++ cryptography engine.

## Tech Stack

- **Runtime**: Go 1.21+
- **Concurrency**: Goroutines, channels
- **Encryption Engine**: C++17, OpenSSL (AES-256)
- **IPC**: Subprocess communication, pipes
- **File Monitoring**: Linux inotify
- **Platform**: Fedora Linux
- **Build Tools**: Make, GCC/Clang

## Getting Started

### Prerequisites

- Go 1.21 or later
- C++ compiler (g++ or clang++)
- OpenSSL development libraries (`openssl-devel` on Fedora)
- Linux kernel with inotify support

### Building

```bash
# Clone the repository
git clone https://github.com/Dtv48-olu/encryption-daemon.git
cd encryption-daemon

# Build the C++ encryption engine
make build-crypto

# Build the Go daemon
go build -o encryption-daemon ./cmd/daemon

# Install the daemon (optional)
sudo make install
```

### Running the Daemon

```bash
# Start monitoring a directory with automatic encryption
./encryption-daemon --watch /path/to/directory --algorithm aes256

# As a systemd service (post-installation)
sudo systemctl start encryption-daemon
sudo systemctl enable encryption-daemon

# View daemon logs
journalctl -u encryption-daemon -f
```

## Architecture

```
encryption-daemon/
├── cmd/
│   └── daemon/           # Main Go application entry point
├── internal/
│   ├── watcher/          # Linux inotify directory monitoring
│   ├── crypto/           # C++ AES-256 engine bindings
│   └── config/           # Configuration management
├── crypto/
│   ├── engine.cpp        # High-performance encryption implementation
│   ├── engine.h
│   └── Makefile
├── Makefile              # Build orchestration
└── systemd/
    └── encryption-daemon.service
```

## Key Features (Roadmap)

- [ ] Real-time directory monitoring with inotify
- [ ] Concurrent file processing with worker pool
- [ ] AES-256 encryption with secure key management
- [ ] Progress tracking and logging
- [ ] Systemd service integration
- [ ] Performance metrics and benchmarking

## Configuration

```bash
# Environment variables
ENCRYPTION_KEY_PATH=/path/to/keyfile
ENCRYPTION_LOG_LEVEL=debug
WORKER_THREADS=4
```

## Performance

Target: Process 1GB+ of files per minute on modern hardware.

## License

MIT License
