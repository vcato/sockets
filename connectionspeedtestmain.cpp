#include <iostream>
#include "socket.hpp"

using std::cerr;


static const int port = 4782;
static const int n_iterations = 1000;

namespace {
class Server {
  public:
    Server(int port);
    ~Server();

    void waitForConnection();
    void waitForClientToClose();
    void closeConnection();

  private:
    Socket listen_socket;
    Socket data_socket;

    void startListening(int port);
    void stopListening();
    void acceptConnection();
};


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
}


static void runServer()
{
  Server server(port);

  for (int i=0; i!=n_iterations; ++i) {
    server.waitForConnection();
    server.waitForClientToClose();
    server.closeConnection();
    cerr << ".";
  }

  cerr << "\n";
}


static void runClient()
{
  Socket socket;

  for (int i=0; i!=n_iterations; ++i) {
    socket.connectTo("localhost",port);
    socket.close();
    cerr << ".";
  }

  cerr << "\n";
}


int main(int argc,char **argv)
{
  Socket::initialize();

  if (argc!=2) {
    cerr << "Usage: " << argv[0] << " (server|client)\n";
    return EXIT_FAILURE;
  }

  std::string operation = argv[1];

  if (operation=="server") {
    runServer();
    return EXIT_SUCCESS;
  }

  if (operation=="client") {
    runClient();
    return EXIT_SUCCESS;
  }

  cerr << "Unknown operation: " << operation << "\n";
  return EXIT_FAILURE;
}
