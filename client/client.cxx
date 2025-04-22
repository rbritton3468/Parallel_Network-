#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <chrono>
#include <atomic>

// g++ -std=c++20 -I. -pthread processServer.cxx runEXE/runEXE.cxx -lcurl -o processServe
uint16_t ServerPort = 9000;
uint16_t WaitPort = 9001;


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
        return findOpenPort(port+1);
        
    }

}

std::atomic<bool> timeOut = true;

void TimOutTimer(){
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if(timeOut.load()){
        std::cout << "Timeout Occurred" << std::endl;
        exit(1);
    }
}



int main(int argc, char* argv[]) {
    std::string exeCMD;
    if(argc < 3) { std::cerr << "usage: peer_a <peer‑ip> <exe>\n"; return 1; }
    for(int i = 2; i < argc; i++) {
        exeCMD += argv[i];
        exeCMD += '\n';
    }

    boost::asio::ip::udp::endpoint serverEndPoint(boost::asio::ip::make_address(argv[1]), ServerPort);
    boost::asio::ip::udp::endpoint serverWaitPoint(boost::asio::ip::make_address(argv[1]), WaitPort);

    std::string result;
    std::array<char, 1024> buf;

    boost::asio::ip::udp::socket sock = findOpenPort(1);
    sock.send_to(boost::asio::buffer(exeCMD), serverEndPoint);
    std::thread timerThread(TimOutTimer);//Process Server has 5 seconds to send the initialization message or timeout occurs
    timerThread.detach();
    sock.receive_from(boost::asio::buffer(buf), serverEndPoint);
    timeOut.store(false);//starts timeout


    for(int i = 2; i < argc; i++){
        auto n = sock.receive_from(boost::asio::buffer(buf), serverEndPoint);

        std::cout<<std::string(std::string_view(buf.data(), n))<< std::endl;
        sock.send_to(boost::asio::buffer("NEXT RESULT"), serverWaitPoint);
    }
   



    return 0;
}