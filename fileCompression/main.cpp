#include "compression.hpp"
#include <string>
#include <iostream>

int main(){
    compressFile("testFile");
    decompressFile("testFile.cmp");
    return 0;
}