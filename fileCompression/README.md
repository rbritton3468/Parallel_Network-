# fileCompressor: .exe file compressing and decompressing

This is our code for compressing and decompressing binary executable files. We created this to be portable and easy
to use.

---

# Overall Structure:

1. main.cpp
    Parses the command-line argument given and routes it to either the compression or decompression code

2. compression.cpp/compression.h  
    These files contain compressFile(): this reads the input file, compresses the contents, and writes a compressed binary version
    Also contains decompressFile(): this decompresses a given file, and writes the same file that was initially compressed

