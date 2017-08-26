#include "client.hpp"


void Client::connectTo(const std::string &hostname,int port)
{
  data_socket.connectTo(hostname,port);
}


void Client::closeConnection()
{
  data_socket.close();
}
