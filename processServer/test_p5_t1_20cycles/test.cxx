#include <iostream>
#include <thread>

std::string exe = "./t1_test";

#define N 24

bool result[N];
std::thread thread[N];

std::string correctResult ="""[child] count match\n[child] count ok\n*** arenas:40\n[parent] child terminated with status 0\n[parent] count match\n[parent] count ok\n*** arenas:40";


void runCommand(std::string command,int resultNum){
    std::string output;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to execute the following command : " << command << std::endl;
        result[resultNum]=false;
        return;
    }
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    auto returnCode = pclose(pipe);
    if(output.compare(correctResult)==0)result[resultNum]=true;
    else result[resultNum]=false;
}

int main() {
    for(int i =0;i<N;i++){
        thread[i] = std::thread(runCommand,exe,i);
    }
    for(int i =0;i<N;i++){
        thread[i].join();
    }
    for(int i =0;i<N;i++){
        if(result[i]==0){
            std::cout<<"FAIL"<<std::endl;
            exit(0);
        }
    }

    std::cout<<"PASS"<<std::endl;
    return 0;
}


