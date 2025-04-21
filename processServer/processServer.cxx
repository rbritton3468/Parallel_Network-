#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>






int main(int argc, char* argv[])
{
    if(argc < 3) {
        std::cerr << "usage: peer_a <peer‑ip> <peer-port>\n";
        return 1;
    }
    
    std::cout<<"Process Server ID: " << argv[1] << " Port: " << argv[2] << std::endl;
    std::string initMessage;
    initMessage = argv[1];
    initMessage += '\n';
    initMessage += argv[2];
    initMessage += '\n';

    uint16_t RecevePort = std::stoi(argv[2]);

    boost::asio::io_context io;
    boost::asio::ip::udp::socket sock(io, {boost::asio::ip::udp::v4(), RecevePort});

    std::array<char, 1024> buf;
    boost::asio::ip::udp::endpoint initServer(boost::asio::ip::make_address("127.0.0.1"), 9999);


    sock.send_to(boost::asio::buffer(initMessage), initServer);

    boost::asio::ip::udp::endpoint sender(boost::asio::ip::make_address("127.0.0.1"), RecevePort);
    for (;;){
        auto n = sock.receive_from(boost::asio::buffer(buf), sender);
        std::cout << "\n<managerServer> " << std::string_view(buf.data(), n) << '\n';

    }

}