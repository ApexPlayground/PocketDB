
# PocketDB

A lightweight, Redis-compatible in-memory key-value database built from scratch in C++17.

## Features

- TCP server with multithreaded client handling
- Redis Serialization Protocol (RESP) support
- String, list, and hash data types
- Key expiration (TTL)
- Periodic disk persistence
- Thread-safe database operations

## Build & Run

```bash
make
./pocketdb
```

Default port: `6379`

Custom port:

```bash
./pocketdb 6380
```

Connect using `redis-cli`:

```bash
redis-cli -p 6379
```

## Tech Stack

C++17, POSIX Sockets, STL, Makefile
