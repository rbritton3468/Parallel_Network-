#include <string>


class Process{
    std::string processName;
    std::string processPath;
    std::string processArgs;
    std::string processStatus;
    size_t processID;
    size_t runTime;


};


class ProcessServer{
    private:
        int ServerID;
        Process* currentProcess;

    public:
        std::string process(Process process){
            // Add process to the server
        }

};






int main(int argc, char* argv[]) {

    
    return 0;
}