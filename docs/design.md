# synccli Design

## Overview

synccli is a C++17 command-line tool that synchronizes files from a source directory to a destination directory. It supports dry-run, mirror mode, and include/exclude filters using glob-like patterns.

## Modules

- `cli` — lightweight argument parsing without external dependencies.
- `filters` — converts glob patterns to regex and decides whether a relative path should be included.
- `sync` — core engine: traverses the source, copies/overwrites files, and optionally mirrors deletions.
- `utils` — helpers for path normalization, directory creation, and file comparison.

## Key Behaviors

- Include rules (if provided) restrict the sync set; exclude rules remove matches after includes are applied.
- Dry-run prints planned actions without touching the filesystem.
- Mirror mode deletes destination files that are not present in the (filtered) source set.

## Future Improvements

- Support for more glob features (e.g., `**`).
- Parallel copy for large trees.
- Progress reporting and verbosity levels.
- Preserve permissions and metadata beyond timestamps.
- Robust error handling/reporting with exit codes per failure class.
