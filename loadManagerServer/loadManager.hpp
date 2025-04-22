#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <list>
#include <mutex>



class ProcessorList;                              
class Process;                                
class ProcessServer;                            
class LoadManager;                             
class Client; 

template <typename T>
class SafeQueue;                                  
                   
size_t write_callback(char *contents, size_t size,
size_t nmemb, std::string *response); // functions
void ProcessServerInitialization();
void processClient(Client clinet);
void processExecutable(std::string path,ProcessServer* server,Client* client);
void clientAccept();