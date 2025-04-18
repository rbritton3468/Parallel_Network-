#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <list>
#include <mutex>
#include "loadManager.hpp"


constexpr uint16_t ClientReceivePort = 9000;
constexpr uint16_t InitializationPort = 9999;
constexpr uint16_t MY_PORT = 9001;


std::vector<ProcessServer*> processServers;



class ProcessorList{
    private:
        std::mutex mtx;
        std::list<std::string> list;
    public:
        void add(std::string item){
            std::lock_guard<std::mutex> lock(mtx);
            list.push_back(item);
        }

        void remove(std::string item){
            std::lock_guard<std::mutex> lock(mtx);
            list.remove(item);
        }

        bool contains(std::string item){
            std::lock_guard<std::mutex> lock(mtx);
            return std::find(list.begin(), list.end(), item) != list.end();
        }

};

class Process{
    std::string processName;
    std::string processPath;
    std::string processArgs;
    std::string processStatus;
    size_t processID;
    size_t runTime;
};


class ProcessServer{
    public:
        uint16_t receivePort;
        uint16_t sendPort;
        std::string IPAddress;
        boost::asio::io_context io;
        uint16_t processServerID;
        boost::asio::ip::udp::socket socket;
        boost::asio::ip::udp::endpoint serverEndPoint;

        void sendMessage(std::string message){
            socket.send_to(boost::asio::buffer(message), serverEndPoint);

        }
        
        ProcessServer(uint16_t id,std::string ip, uint16_t sendPort,uint16_t receivePort):
        processServerID(id), IPAddress(ip), receivePort(receivePort), sendPort(sendPort),

        socket(io, {boost::asio::ip::udp::v4(), receivePort}), // Properly initialize the socket
        serverEndPoint(boost::asio::ip::make_address(IPAddress), sendPort) { // Properly initialize the endpoint
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


            // Add process to the server
            // This function will be called when a request is received from the client
            // It will parse the request and call the appropriate function
            // For example, if the request is to add a process, it will call the addProcess function
            return "";
        }
};

CURL *curl = curl_easy_init();

size_t write_callback(char *contents, size_t size, size_t nmemb, std::string *response) {
    size_t total_size = size * nmemb;
    response->append(contents, total_size);
    return total_size;
}

void ProcessServerInitialization(){
    uint16_t idCounter = 0;
    boost::asio::io_context io;
    boost::asio::ip::udp::socket InitializationSocket(io, {boost::asio::ip::udp::v4(), InitializationPort});

    std::array<char, 1024> buf;
    boost::asio::ip::udp::endpoint sender;
    while (true)
    {
        auto n = InitializationSocket.receive_from(boost::asio::buffer(buf), sender);
        std::string processServerdata = std::string(std::string_view(buf.data(), n));

        std::string ip = processServerdata.substr(0, processServerdata.find('\n'));
        std::string processServerPort = processServerdata.substr(processServerdata.find('\n')+1);
        uint16_t processServerPortInt = std::stoi(processServerPort);

        ProcessServer* processServer = new ProcessServer(idCounter,ip,processServerPortInt,9002+idCounter);
        std::cout<<"New Process Server ID: " << idCounter << " IP: " << ip << " Port: " << processServerPort<< std::endl;
        processServers.push_back(processServer);
        idCounter++;
    }
    
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
    if(argc < 0) {
        std::cerr << "Usage: " << argv[0] << " <server_address>"<< " <exe>" << std::endl;
        return 1;
    }
    std::thread initializationThread(ProcessServerInitialization);
    //boost::asio::io_context io;
    //boost::asio::ip::udp::socket sock(io, {boost::asio::ip::udp::v4(), MY_PORT});
    //boost::asio::ip::udp::endpoint peer(boost::asio::ip::make_address("127.0.0.1"), ClientReceivePort);

    std::string line;
    //std::cout << "> " << std::flush;
    while (std::getline(std::cin, line))
    {
    std::cout << "> " << std::flush;
    for(ProcessServer* processServer : processServers){
        processServer->sendMessage(line);
    }


}
    return 0;
}