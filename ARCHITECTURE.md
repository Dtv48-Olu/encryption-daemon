# Architecture — encryption-daemon

## Overview

`encryption-daemon` is a concurrent Linux daemon written in Go that monitors user-specified directories
and automatically encrypts new files using a high-performance C++ encryption engine backed by libsodium.
A companion CLI tool (`encryption-ctl`) communicates with the running daemon over a Unix domain socket.

---

## Design Philosophy

| Decision | Rationale |
|---|---|
| **Go for the daemon** | Goroutines make it trivial to watch many directories concurrently without the overhead of OS threads. Go's standard library also has first-class support for Unix sockets, signals, and systemd integration. |
| **C++ for the encryption engine** | Encryption is a hot path. C++ gives direct access to libsodium's native API with no FFI overhead. |
| **libsodium over OpenSSL** | libsodium is harder to misuse. It handles nonce generation, padding, and key sizing correctly by design. The API surface is small and opinionated. |
| **Functional C++ (no OOP)** | The engine is a pure function library — no classes, no file I/O, no global state. This makes it trivially testable and keeps the language boundary clean. |
| **Subprocess IPC (pipes)** | Calling C++ from Go via cgo introduces build complexity and makes sanitizers difficult to use on both sides. A subprocess boundary keeps each language's toolchain independent and failures contained. |
| **Linux only** | The watcher relies on inotify, which is a Linux kernel interface. This is an intentional constraint, not an oversight. |

---

## Components

### 1. CLI Tool — `encryption-ctl`
The user-facing binary, written in Go. Sends commands to the daemon over a Unix domain socket and
prints responses. Stateless — it does not read the daemon config directly.

**Commands:** `start`, `stop`, `status`, `watch <dir>`, `unwatch <dir>`, `encrypt-now <dir>`

---

### 2. Go Daemon
The main background process. Owns the Unix socket listener, dispatches CLI commands, manages the
lifecycle of directory watchers, and coordinates the IPC bridge. Runs as a systemd service under a
dedicated low-privilege user.

---

### 3. Directory Watcher (`internal/watcher`)
Wraps Linux inotify via `fsnotify`. Each watched directory gets its own goroutine. Emits file-creation
events to a shared channel consumed by the daemon. Symlinks are not followed.

---

### 4. IPC Bridge (`internal/crypto`)
Manages the C++ engine subprocess. Serializes encryption requests as JSON over stdin and reads JSON
responses from stdout. Handles subprocess restarts on crash. The bridge is the only component that
communicates with the C++ engine.

---

### 5. C++ Encryption Engine (`crypto/`)
A pure-function library. No file I/O, no user interaction, no global state.
Receives plaintext over stdin and returns ciphertext over stdout.

**Functions:**
- `encryptData()` — encrypts and authenticates a buffer with a given key and nonce using libsodium XChaCha20-Poly1305
- `decryptData()` — decrypts a buffer
- `generateKey()` — generates a random 256-bit key via libsodium
- `generateNonce()` — generates a random nonce
- `zeroMemory()` — overwrites a buffer with zeros (`sodium_memzero`)

---

### 6. Config (`internal/config`)
Reads and writes daemon state (watched directories) to `~/.encryption-daemon/state.json`.
Loaded once at daemon startup. Persisted on every `watch` / `unwatch` command.

---

## Data Flow

```
  User
   |
   |  CLI command (e.g. "watch /home/user/docs")
   v
encryption-ctl
   |
   |  Unix domain socket (/run/encryption-daemon/daemon.sock)
   v
Go Daemon
   |
   |  registers directory
   v
Directory Watcher (goroutine per dir)
   |
   |  inotify CREATE event (new file detected)
   v
Go Daemon (receives event from watcher channel)
   |
   |  sends encrypt request (JSON over stdin pipe)
   v
IPC Bridge
   |
   |  spawns / communicates with subprocess
   v
C++ Encryption Engine (libsodium)
   |
   |  returns ciphertext (JSON over stdout pipe)
   v
IPC Bridge
   |
   |  writes <filename>.enc to disk
   |  deletes original file
   v
  Done
```

---

## Communication Model

### CLI -> Daemon (Unix Socket)

The CLI connects to `/run/encryption-daemon/daemon.sock`, sends a newline-delimited JSON command,
and reads a JSON response before closing the connection.

```
Request:   { "command": "watch", "path": "/home/user/docs" }
Response:  { "ok": true, "message": "Now watching /home/user/docs" }
```

### Daemon -> C++ Engine (Subprocess Pipes)

The IPC bridge keeps a single long-lived C++ engine subprocess. Requests and responses are
newline-delimited JSON written to stdin and read from stdout.

```
Request:   { "op": "encrypt", "key": "<hex>", "nonce": "<hex>", "data": "<base64>" }
Response:  { "ok": true, "ciphertext": "<base64>" }
```

The engine never touches the filesystem. All I/O is owned by the daemon.

---

## Security Considerations

- **Least privilege** — daemon runs as a dedicated `encryption-daemon` user with explicit permissions on watched directories only
- **Key hygiene** — keys are never written to logs or the state file. `zeroMemory()` is called immediately after each operation
- **No double encryption** — files with `.enc` extension are silently skipped
- **Symlink rejection** — the watcher does not follow symlinks
- **Subprocess isolation** — a crash in the C++ engine cannot corrupt daemon state

---

## Project Structure

```
encryption-daemon/
├── cmd/
│   └── daemon/           # Go daemon entry point (main.go)
├── internal/
│   ├── watcher/          # inotify directory monitoring
│   ├── crypto/           # IPC bridge to C++ engine
│   └── config/           # JSON state persistence
├── crypto/
│   ├── include/
│   │   └── engine.h      # C++ engine public header
│   └── src/
│       └── engine.cpp    # C++ engine implementation (libsodium)
├── systemd/
│   └── encryption-daemon.service
├── docs/                 # Additional diagrams and references
├── CMakeLists.txt        # C++ engine build
├── Makefile              # Orchestrates CMake + go build
├── README.md
└── ARCHITECTURE.md
```

---

## Building

**Prerequisites:** Go >= 1.21, CMake >= 3.16, libsodium-dev, C++17 compiler

```sh
# Build everything
make

# Build only the C++ engine
make engine

# Build only the Go daemon and CLI
make go

# Run tests
make test

# Install systemd service (requires root)
make install
```
