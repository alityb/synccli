# synccli

A fast, lightweight file synchronization tool written in C++17. Perfect for local backups, project syncing, and keeping directories in sync without the overhead of rsync.

## What it does

synccli recursively copies files from a source directory to a destination, but only when they've actually changed. It's like `cp -r` but smart - it skips files that are already identical, making subsequent runs much faster. Think of it as a local rsync without the complexity.

## Features

- **Fast incremental sync** - Only copies changed files
- **Dry-run mode** - See what would happen before making changes
- **Mirror mode** - Remove stale files from destination
- **Smart filtering** - Include/exclude with glob patterns
- **Performance metrics** - Built-in timing and throughput
- **No external dependencies** - Pure C++17 with std::filesystem

## Performance

Here's how synccli stacks up against the usual tools:

| Tool | Fresh Copy | Incremental | Memory Usage |
|------|------------|-------------|--------------|
| **synccli** | ⚡ 0.03s | ⚡ 0.01s | 3840 kB |
| cp | 0.03s | 0.02s (copies all) | 2560 kB |
| rsync | 0.09s | 0.04s | 6912 kB |

### Build

```bash
git clone https://github.com/alityb/synccli
cd synccli

# With CMake (recommended)
cmake -S . -B build
cmake --build build -j$(nproc)

# Or direct compile
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude \
  src/*.cpp -o build/synccli
```

### Basic Usage

```bash
# Simple sync
./build/synccli -s ~/Documents -d ~/backup

# Preview changes first
./build/synccli -s ~/Documents -d ~/backup --dry-run

# Mirror mode (delete stale files)
./build/synccli -s ~/Documents -d ~/backup --mirror

# Show timing info
./build/synccli -s ~/Documents -d ~/backup --time
```

## Common Use Cases

### Backup Documents
```bash
./build/synccli -s ~/Documents -d /mnt/backup/Documents \
  --exclude "*.log" \
  --exclude "temp/" \
  --exclude ".DS_Store" \
  --time
```

### Sync Project Code
```bash
./build/synccli -s ~/Projects/myapp -d ~/backup/myapp \
  --exclude "node_modules/" \
  --exclude "*.log" \
  --exclude "build/" \
  --mirror \
  --time
```

### Filter by File Type
```bash
# Only sync specific file types
./build/synccli -s ~/Documents -d ~/backup \
  --include "*.pdf" \
  --include "*.doc*" \
  --exclude "*.tmp"
```

## How It Works

1. **Scan Source**: Recursively traverses source directory
2. **Apply Filters**: Uses include/exclude glob patterns
3. **Check Changes**: Compares file size and modification time
4. **Copy Files**: Only transfers changed or new files
5. **Mirror Cleanup**: Optionally removes stale destination files

### Filter Logic
- **Include patterns**: If specified, only matching files are considered
- **Exclude patterns**: Applied after includes to remove unwanted files
- **Glob support**: `*` (any characters), `?` (single character), `/` (directory separator)

## Permissions & Mounts

### Root-owned Destinations
If syncing to system directories (e.g., `/mnt/backup`), you may need `sudo`:

```bash
sudo ./build/synccli -s ~/Documents -d /mnt/backup/Documents
```

**⚠️ Warning**: Files created with `sudo` will be owned by root. Fix ownership after:
```bash
sudo chown -R "$USER:$USER" /mnt/backup/Documents
```

### External Drives
Ensure external drives are properly mounted:
```bash
# Check if mounted
findmnt /mnt/backup

# Mount if needed
sudo mount /dev/sdXN /mnt/backup  # Replace sdXN with your device
```

### User Paths with Sudo
When using `sudo`, avoid `~` and `$HOME` (they point to root's home):
```bash
# ❌ Wrong
sudo ./build/synccli -s ~/Documents -d /mnt/backup

# ✅ Correct
sudo ./build/synccli -s "/home/$SUDO_USER/Documents" -d /mnt/backup
```

## Benchmarking

Want to test it yourself? Here's how:

```bash
# Create test dataset
mkdir -p /tmp/benchmark
for i in {1..1000}; do echo "Content $i" > "/tmp/benchmark/file_$i.txt"; done

# Test fresh copy
echo "=== FRESH COPY ==="
time cp -r /tmp/benchmark /tmp/cp_test/
time rsync -a /tmp/benchmark/ /tmp/rsync_test/
time ./build/synccli -s /tmp/benchmark -d /tmp/synccli_test --time

# Test incremental (no changes)
echo "=== INCREMENTAL SYNC ==="
time cp -r /tmp/benchmark /tmp/cp_test/      # Copies everything
time rsync -a /tmp/benchmark/ /tmp/rsync_test/  # Skips unchanged
time ./build/synccli -s /tmp/benchmark -d /tmp/synccli_test --time  # Skips unchanged
```

### Expected Results
- **Fresh copy**: synccli ≈ cp < rsync
- **Incremental**: synccli < rsync < cp
- **Memory usage**: cp < synccli < rsync

## Output Examples

### Dry-run with Mirror
```bash
$ ./build/synccli -s ~/Documents -d ~/backup --dry-run --mirror --time

[DRY RUN] Would copy: ~/Documents/new_file.txt → ~/backup/new_file.txt
[DRY RUN] Would overwrite: ~/Documents/updated.txt → ~/backup/updated.txt
[DRY RUN] Would delete: ~/backup/old_file.txt
[SUMMARY] 1 files would be copied, 1 files would be overwritten, 1 files would be deleted.
[TIMING] Duration: 45 ms, Transferred: 0.05 MiB, Throughput: 1.11 MiB/s
```

### Real Sync with Timing
```bash
$ ./build/synccli -s ~/Documents -d ~/backup --time

[SUMMARY] Copied: 15, Overwritten: 3, Deleted: 0, Skipped: 1247
[TIMING] Duration: 127 ms, Transferred: 2.34 MiB, Throughput: 18.43 MiB/s
```

## Troubleshooting

### Common Issues

**Permission Denied**
```bash
# Destination needs write permissions
sudo chmod 755 /mnt/backup
# Or use a user-writable directory
./build/synccli -s ~/Documents -d ~/backup
```

**Files Not Syncing**
- Check include/exclude patterns
- Verify source path exists and is readable
- Use `--dry-run` to preview what would happen

**Slow Performance**
- Use `--time` to measure actual performance
- Consider excluding large directories (e.g., `node_modules/`, `build/`)
- Ensure destination is on fast storage (not network/USB 2.0)

### Debug Mode
For verbose output, you can modify the source code to add debug logging or use system tools:
```bash
# Monitor file operations
strace -e trace=file ./build/synccli -s ~/Documents -d ~/backup 2>&1 | grep -E "(open|write|copy)"

# Check what files are being processed
./build/synccli -s ~/Documents -d ~/backup --dry-run | grep "Would"
```

## Project Structure

```
synccli/
├── CMakeLists.txt          # Build configuration
├── include/                # Header files
│   ├── cli.hpp            # Command-line parsing
│   ├── sync.hpp           # Core sync engine
│   ├── filters.hpp        # Include/exclude logic
│   └── utils.hpp          # Helper functions
├── src/                   # Source files
│   ├── main.cpp           # Entry point
│   ├── cli.cpp            # CLI implementation
│   ├── sync.cpp           # Sync engine
│   ├── filters.cpp        # Filtering logic
│   └── utils.cpp          # Utilities
├── tests/                 # Test suite
└── docs/                  # Documentation
```

## Limitations & Future Work

### Current Limitations
- Basic glob patterns (`*`, `?`) - no `**` recursive matching
- Single-threaded file operations
- Limited metadata preservation (timestamps only)
- No network/remote sync capabilities

### Planned Features
- [ ] Recursive glob patterns (`**/*.txt`)
- [ ] Parallel file operations
- [ ] Progress bars and verbosity levels
- [ ] Extended attribute preservation
- [ ] Network sync support
- [ ] Configuration files

### Development Setup
```bash
# Build and run tests
cmake -S . -B build
cmake --build build -j$(nproc)
./build/synccli_tests

# Run with debug info
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

## License

This project is licensed under the MIT License.