# 48430-Fundamental-of-C-Programming
Assignment3GroupProject.c file path

Project Management Tool: Kandan Board: https://student791197.monday.com/boards/2074235491

Group Report: https://docs.google.com/document/d/1yt3ZejDWo48YKWHe_kk6SP53gmFtzmzGuz5XfA1wd2o/edit?usp=sharing \
Checkpoint 1 Report: https://docs.google.com/document/d/1Mdtk8SHiuubqkFZEL1-84JyegKBcj8k0LFYuVrKjSK0/edit?tab=t.0 

Architecture:

*locker.c (main and interactive menu) - Alex (to be done)

*locker.h (public prototypes and typedefs) - Alex (to be done)

*crypto.c/ crypto.h (PRNG; encrypt/decrypt helpers, and key derivation) - Edward (to be done)

*compress.c/ compress.h (compress/ decompress) - Tarun (to be done)

*storage.c storage.h (file format, read, write, index management) - Ed M (to be done)

*utils.c /utils.h (string helpers, I/O wrappers, error handling) - Tarun (to be done)

*makefile (function for building) - Tarun (to be done)

## Build

Using make (Windows PowerShell):

```
make
```

Debug build (adds -DDEBUG):

```
make debug
```

Run:

```
./locker   (or .\\locker.exe on Windows)
```

## Modules

- `locker.h` / `locker.c`: Public API + core operations (open, add, extract, list, search, remove, change PIN). Currently contains stubs for later implementation.
- `compress.h` / `compress.c`: Simple Run-Length Encoding (RLE) compression/decompression.
- `crypto.h` / `crypto.c`: Simple XOR-based cipher with naive key derivation from PIN (placeholder for enhancement).
- `storage.h` / `storage.c`: Placeholder for persistence of index + data (to be implemented).
- `util.h` / `util.c`: Utility helpers for file I/O and a placeholder timestamp.
- `main.c`: Interactive menu driver.

## Next Steps (Checkpoint Roadmap)
1. Implement real locker file format (header + entries + data region).
2. Integrate compression & encryption inside `lockerAddFile` and reverse process in `lockerExtractFile`.
3. Persist master PIN securely (store hashed/obfuscated form) and re-encrypt on PIN change.
4. Add sorting modes & advanced search (substring already stubbed).
5. Robust input validation & error codes for resilience.

## Notes
Only standard headers allowed: stdio.h, stdlib.h, string.h, math.h. The current code adheres to this (pedantic flags enabled). Additional algorithms (e.g., alternative compression or searching structures) can be layered without external libraries.
