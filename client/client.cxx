#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <chrono>
#include <atomic>

// g++ -std=c++20 -I. -pthread processServer.cxx -lcurl -o processServer
uint16_t ServerPort = 9000;

uint16_t MY_PORT= 8001;

std::atomic<bool> timeOut = true;

void TimOutTimer(){
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if(timeOut.load()){
        std::cout << "Timeout Occurred" << std::endl;
        exit(1);
    }
}

boost::asio::ip::udp::endpoint serverEndPoint(boost::asio::ip::make_address("127.0.0.1"), ServerPort);

int main(int argc, char* argv[]) {
    std::string exeCMD;
    if(argc < 3) { std::cerr << "usage: peer_a <peer‑ip> <exe>\n"; return 1; }
    for(int i = 2; i < argc; i++) {
        exeCMD += argv[i];
        exeCMD += '\n';
    }

    std::string result;
    std::array<char, 1024> buf;
    boost::asio::io_context io;
    boost::asio::ip::udp::socket sock(io, {boost::asio::ip::udp::v4(), MY_PORT});
    sock.send_to(boost::asio::buffer(exeCMD), serverEndPoint);
    std::thread timerThread(TimOutTimer);//Process Server has 5 seconds to send the initialization message or timeout occurs
    timerThread.detach();
    sock.receive_from(boost::asio::buffer(buf), serverEndPoint);
    sock.receive_from(boost::asio::buffer(buf), serverEndPoint);


    result = std::string(std::string_view(buf.data(), buf.size()));
    std::cout << result  << std::flush;



    return 0;
}