#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "socket.hpp"


using std::cerr;
using std::string;
using namespace std::chrono_literals;



static void waitForRemoteToClose(Socket &server_socket)
{
  for (;;) {
    char buffer[1];
    ssize_t n_bytes_received = server_socket.recv(buffer,sizeof buffer);

    if (n_bytes_received==0) {
      break;
    }
  }
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
    Socket listen_socket;

    listen_socket.bindTo(/*port*/1234);
    listen_socket.startListening(/*backlog*/1);

    Socket server_socket;
    server_socket.acceptFrom(listen_socket);
    server_socket.setNoDelay(true);

    for (int i=0; i!=1000; ++i) {
      char buffer[1] = {'.'};
      server_socket.send(buffer,sizeof buffer);
      std::this_thread::sleep_for(5ms);
    }

    waitForRemoteToClose(server_socket);

    server_socket.close();
    listen_socket.close();
  }
  else if (mode=="client") {
    if (argc<3) {
      cerr << "No hostname specified.\n";
      return EXIT_FAILURE;
    }

    string hostname = argv[2];

    Socket client_socket;
    client_socket.connectTo(hostname,1234);
    char buffer[1];
    bool first_read = true;
    std::chrono::high_resolution_clock::time_point last_read_time;
    using clock = std::chrono::high_resolution_clock;

    for (;;) {
      ssize_t n_bytes_received =
        client_socket.recv(buffer,sizeof buffer);

      if (n_bytes_received==0) {
        break;
      }

      clock::time_point current_read_time = clock::now();

      if (!first_read) {
        clock::duration d = current_read_time - last_read_time;
        int n_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
        cerr << "n_ms: " << n_ms << "\n";
      }
      else {
        first_read = false;
      }

      last_read_time = current_read_time;
      cerr << buffer[0];
    }
    client_socket.close();
  }
  else {
    cerr << "Unknown mode: " << mode << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
