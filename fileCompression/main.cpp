#include "compression.hpp"
// #include "decompressor.h"
#include <string>
#include <iostream>

int main(){
    compressFile("testFile");
    decompressFile("testFile.cmp");
    return 0;
}