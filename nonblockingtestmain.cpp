#include <iostream>
#include <string>
#include <thread>
#include "socket.hpp"


using std::cerr;
using std::string;
using namespace std::chrono_literals;


static void waitForRemoteToClose(Socket &server_socket)
{
  for (;;) {
    char buffer[1];
    Socket::NonBlockingRecvResult result =
      server_socket.nonBlockingRecv(buffer,sizeof buffer);

    if (!result.would_block) {
      if (result.n_bytes_received==0) {
        break;
      }

      if (result.n_bytes_received<0) {
        cerr << "Error\n";
      }
    }
  }
}


static void runServer()
{
  Socket listen_socket;
  listen_socket.bindTo(/*port*/1234);
  listen_socket.startListening(/*backlog*/1);

  Socket server_socket;
  server_socket.acceptFrom(listen_socket);
  server_socket.setNonBlocking(true);
  waitForRemoteToClose(server_socket);
  server_socket.close();
  listen_socket.close();
}


static void runClient(const string &hostname)
{
  Socket client_socket;
  client_socket.connectTo(hostname,/*port*/1234);
  std::this_thread::sleep_for(1s);
  client_socket.close();
}


int main(int argc,char **argv)
{
  if (argc==1) {
    cerr << "Usage: nodelay_test (server|client <hostname>)\n";
    return EXIT_FAILURE;
  }

  Socket::initialize();

  string mode = argv[1];

  if (mode=="server") {
    runServer();
  }
  else if (mode=="client") {
    if (argc<3) {
      cerr << "No hostname specified.\n";
      return EXIT_FAILURE;
    }

    string hostname = argv[2];
    runClient(hostname);
  }
  else {
    cerr << "Unknown mode: " << mode << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
