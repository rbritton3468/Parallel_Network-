#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <list>
#include <mutex>
#include "loadManager.hpp"

// g++ -std=c++20 -I. -pthread loadManager.cxx -lcurl -o loadManager
constexpr uint16_t ClientReceivePort = 9000;
constexpr uint16_t InitializationPort = 9999;
constexpr uint16_t MY_PORT = 9001;

boost::asio::io_context io;
boost::asio::ip::udp::socket Clientsocket(io, {boost::asio::ip::udp::v4(), ClientReceivePort});

std::vector<ProcessServer*> processServers;
std::vector<Client*> Clients;


class ClientMap{
    public:
        std::mutex mtx;
        std::unordered_map<std::string, Client*> map;
    void addProcess(std::string ip){


    }
 


};


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
    public:
        std::string processName;
        std::string processID;
        std::string processPath;
        std::string processArguments;
        std::string processStatus;
        std::string processServerID;
        std::string processServerIP;
        uint16_t processServerPort;

        Process(std::string name, std::string id, std::string path, std::string arguments, std::string status, std::string serverID, std::string serverIP, uint16_t serverPort):
        processName(name), processID(id), processPath(path), processArguments(arguments), processStatus(status), processServerID(serverID), processServerIP(serverIP), processServerPort(serverPort) {
        }
};

class Client{
    
    public:

    uint16_t sendPort;
    std::string IPAddress;
    boost::asio::io_context io;
    size_t clientID;
    std::queue<std::string> ProcessQueue;
    std::queue<std::string> resultQueue;
    boost::asio::ip::udp::endpoint clientEndPoint;
    
        Client(size_t id,std::string ip, uint16_t sendPort):
        clientID(id), IPAddress(ip),sendPort(sendPort),
        clientEndPoint(boost::asio::ip::make_address(ip), sendPort) // Properly initialize the endpoint
        {}
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
    
    while (true)
    {
        boost::asio::ip::udp::endpoint sender;
        auto n = InitializationSocket.receive_from(boost::asio::buffer(buf), sender);

        
        std::string processServerdata = std::string(std::string_view(buf.data(), n));
        
        std::string ip = sender.address().to_string();
        uint processServerPort = sender.port();



        ProcessServer* processServer = new ProcessServer(idCounter,ip,processServerPort,9002+idCounter);
        processServer->sendMessage("Process Server Initialized");
        

        std::cout<<"New Process Server ID: " << idCounter << " IP: " << ip << " Port: " << processServerPort<< std::endl;
        
        processServers.push_back(processServer);
        idCounter++;
    }
} 

void clientAccept(){
    size_t clientID = 0;

    while(true){
        std::array<char, 1024> buf;
        boost::asio::ip::udp::endpoint clientEndPoint;
        auto n = Clientsocket.receive_from(boost::asio::buffer(buf), clientEndPoint);
        uint16_t clientPort = clientEndPoint.port();
        std::string clientIP = clientEndPoint.address().to_string();
        Clientsocket.send_to(boost::asio::buffer("Client Initialized"), clientEndPoint);
    
        std::string clientdata = std::string(std::string_view(buf.data(), n));
        std::cout << "New Client ID: " << clientID << " IP: " << clientIP << " Port: " << clientPort<< std::endl;
        Client* client = new Client(clientID,clientIP,clientPort);
        client->ProcessQueue.push(clientdata);
        Clients.push_back(client);
        clientID++;
        // Process the client data
        // This function will be called when a request is received from the client
        // It will parse the request and call the appropriate function

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
    std::thread clientThread(clientAccept);

    std::string line;

    while (std::getline(std::cin, line))
    {
    std::cout << "> " << std::flush;
    for(ProcessServer* processServer : processServers){
        processServer->sendMessage(line);
    }


}
    return 0;
}