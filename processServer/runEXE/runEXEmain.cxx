#include <iostream>
#include "runEXE.hpp"


extern std::string runCommandAndGetOutput(std::string command);

int main(){

    std::cout<<runCommandAndGetOutput("cat runEXE.cxx")<<std::endl;

    return 0;
}