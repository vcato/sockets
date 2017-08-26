#include "server.hpp"


Server::Server(int port)
{
  startListening(port);
}


Server::~Server()
{
  stopListening();
}


void Server::startListening(int port)
{
  int backlog = 1;

  listen_socket.bindTo(port);
  listen_socket.startListening(backlog);
}


void Server::stopListening()
{
  listen_socket.close();
}


void Server::acceptConnection()
{
  data_socket.acceptFrom(listen_socket);
}


void Server::waitForConnection()
{
  acceptConnection();
}


void Server::waitForClientToClose()
{
  for (;;) {
    char buffer[1024];
    int n_read = data_socket.recv(buffer,sizeof buffer);
    if (n_read==0) break;
  }
}


void Server::closeConnection()
{
  data_socket.close();
}
