#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>

uint16_t ServerPort = 9000;

uint16_t MY_PORT= 8001;


boost::asio::ip::udp::endpoint serverEndPoint(boost::asio::ip::make_address("127.0.0.1"), ServerPort);

int main(int argc, char* argv[]) {
    boost::asio::io_context io;
    boost::asio::ip::udp::socket sock(io, {boost::asio::ip::udp::v4(), MY_PORT});


    std::string line;
    //std::cout << "> " << std::flush;
    while (std::getline(std::cin, line))
    {
    std::cout << "> " << std::flush;


    sock.send_to(boost::asio::buffer(line), serverEndPoint);
}
    return 0;
}