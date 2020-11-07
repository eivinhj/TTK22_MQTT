#include "mqtt/async_client.h"

#define DFLT_ADDRESS 127.0.0.1:2312
#define PERSIST_DIR ./

const std::string DEFAULT_SERVER_ADDRESS	{ "tcp://localhost:2312" }; //TODO Change this for server
const std::string DEFAULT_CLIENT_ID		    { "client 1"};


int main(int argc, char *argv[]){
   std::cout << "Hello World!" << std::endl;
   //mqtt::async_client cli(DFLT_ADDRESS, "", 120, PERSIST_DIR);
   mqtt::async_client mqtt_client(DEFAULT_SERVER_ADDRESS, DEFAULT_CLIENT_ID);
   

   return 0;
}