#include <string>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <list>
#include <mutex>



class ProcessorList;                            //│  
class Process;                                  //│
class ProcessServer;                            //│
class LoadManager;                              //│
                   
size_t write_callback(char *contents, size_t size,
size_t nmemb, std::string *response); // functions
void ProcessServerInitialization();
void processRequest(std::string request);
