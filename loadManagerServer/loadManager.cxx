#include <string>
#include <curl/curl.h>
#include <iostream>

class Process{
    std::string processName;
    std::string processPath;
    std::string processArgs;
    std::string processStatus;
    size_t processID;
    size_t runTime;
};

std::string serverAddress;

class ProcessServer{
    private:
        int ServerID;
        Process* currentProcess;

    public:
        std::string process(Process process){
            // Add process to the server
        }
};


class LoadManager{
    private:
        int loadManagerID;
        ProcessServer* processServer;
        std::string serverAddress;
        int serverPort;
    public:
        std::string addProcess(Process process){
            // This function will be called when a request is received from the client
            // It will parse the request and call the appropriate function
            // For example, if the request is to add a process, it will call the addProcess function
        }
};

CURL *curl = curl_easy_init();

size_t write_callback(char *contents, size_t size, size_t nmemb, std::string *response) {
    size_t total_size = size * nmemb;
    response->append(contents, total_size);
    return total_size;
}

void processRequest(std::string request){
    // Process the request
    // This function will be called when a request is received from the client
    // It will parse the request and call the appropriate function
    // For example, if the request is to add a process, it will call the addProcess function

    while(true){
        // Wait for a request
        // This is a placeholder for the actual implementation
        // In a real implementation, this would be replaced with code to wait for a request from the client
    }
}




int main(int argc, char* argv[]) {
    if(argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <server_address>"<< " <exe>" << std::endl;
        return 1;
    }

    serverAddress = argv[1];
    
    return 0;
}
