#include <iostream>
#include "server.hpp"

using std::cerr;


static const int port = 4782;
static const int n_iterations = 1000;


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
