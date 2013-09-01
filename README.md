Anvil Block Finder
================

A rather quick and dirty program to locate all blocks of a specific block ID in
modern minecraft maps.

Requires:
- gcc+make (either mingw or build-essentials)
- zlib
- pthreads

Compiling:
- Just run `make`

Running:
- `./blockfind_anvil [-b <blockid>] [-t <thread count>] <region directory>`
- If no blcok ID is passed, the program will search for bookcases.
- If no thread count is passed (or it's less than 2) the program works entirely in the main thread
