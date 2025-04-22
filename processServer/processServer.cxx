#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <chrono>
#include <atomic>

std::atomic<bool> timeOut = true;

uint16_t  serverPort = 9999;
uint16_t receivePort = 9998;
// g++ -std=c++20 -I. -pthread processServer.cxx -lcurl -o processServer

boost::asio::io_context io;

boost::asio::ip::udp::socket findOpenPort(uint16_t port) {
    try{
        boost::asio::ip::udp::socket sock(io, {boost::asio::ip::udp::v4(), port});
        return sock;
    }
    catch (std::exception& e) {
        if(port == 9000) {
            std::cerr << "No available port found" << std::endl;
            exit(1);
        }
        return findOpenPort(port-1);
        
    }

}




std::string exeCMD(std::string cmd){//will run external command place holder
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return " COMPLETED";
}





void TimeOutTimer(){
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if(timeOut.load()){
        std::cout << "Timeout Occurred" << std::endl;
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2) { std::cerr << "usage: peer_a <peer‑ip>\n"; return 1; }
    
    std::string initMessage ="init";

    boost::asio::ip::udp::socket sock = findOpenPort(receivePort);


    boost::asio::io_context io;
    

    std::array<char, 1024> buf;
    boost::asio::ip::udp::endpoint initServer(boost::asio::ip::make_address(argv[1]), serverPort);


    sock.send_to(boost::asio::buffer(initMessage), initServer);//send initialization message to load manager server
    std::thread timerThread(TimeOutTimer);//Process Server has 5 seconds to send the initialization message or timeout occurs
    timerThread.detach();

    sock.receive_from(boost::asio::buffer(buf), initServer);
    std::cout<<"IP: " << sock.local_endpoint().address() << " Port: " << sock.local_endpoint().port() << "\n";
    std::cout <<"Server IP: "<< argv[1] << " Server Port: " << initServer.port() << std::endl;
    timeOut.store(false);//stops timeout 


    boost::asio::ip::udp::endpoint sender(boost::asio::ip::make_address(argv[1]), initServer.port());



    for (;;){
        auto n = sock.receive_from(boost::asio::buffer(buf), sender);


        
        std::cout << "\n<managerServer> " << std::string_view(buf.data(), n) << '\n';
        sock.send_to(boost::asio::buffer(exeCMD(std::string(std::string_view(buf.data(), n)))), sender);

    }






}


