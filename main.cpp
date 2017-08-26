#include <string>
#include <cassert>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

using std::string;
using std::cerr;


static const int invalid_file_descriptor = -1;


struct Server {
  int listen_socket_file_descriptor;
  int data_socket_file_descriptor;

  Server(int port)
  : listen_socket_file_descriptor(invalid_file_descriptor),
    data_socket_file_descriptor(invalid_file_descriptor)
  {
    startListening(port);
  }

  ~Server()
  {
    stopListening();
  }

  void startListening(int port)
  {
    assert(listen_socket_file_descriptor==invalid_file_descriptor);
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;
    listen_socket_file_descriptor = socket(domain,type,protocol);
    sockaddr_in server_address;
    memset(
      &server_address,
      0,
      sizeof server_address
    );
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    int bind_result =
      bind(
        listen_socket_file_descriptor,
        reinterpret_cast<sockaddr*>(&server_address),
        sizeof server_address
      );
    if (bind_result!=0) {
      cerr << "Unable to bind socket\n";
      return;
    }
    int backlog = 1;
    int listen_result =
      listen(listen_socket_file_descriptor,backlog);
    if (listen_result!=0) {
      cerr << "Unable to listen\n";
      return;
    }
  }

  void stopListening()
  {
    close(listen_socket_file_descriptor);
    listen_socket_file_descriptor = invalid_file_descriptor;
  }

  void acceptConnection()
  {
    sockaddr_in client_address;
    socklen_t client_address_length = sizeof client_address;
    data_socket_file_descriptor =
      accept(
        listen_socket_file_descriptor,
        reinterpret_cast<sockaddr*>(&client_address),
        &client_address_length
      );
    if (data_socket_file_descriptor==-1) {
      cerr << "Failed to accept connection\n";
      data_socket_file_descriptor = invalid_file_descriptor;
    }
  }

  void waitForConnection()
  {
    acceptConnection();
  }

  void waitForClientToClose()
  {
    assert(data_socket_file_descriptor!=invalid_file_descriptor);
    for (;;) {
      char buffer[1024];
      int n_read = read(data_socket_file_descriptor,buffer,sizeof buffer);
      if (n_read==0) break;
    }
  }

  void closeConnection()
  {
    close(data_socket_file_descriptor);
    data_socket_file_descriptor = invalid_file_descriptor;
  }
};


struct Client {
  int data_socket_file_descriptor;

  Client()
  : data_socket_file_descriptor(invalid_file_descriptor)
  {
  }

  void connectTo(const string &hostname,int port)
  {
    assert(data_socket_file_descriptor==invalid_file_descriptor);
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;
    data_socket_file_descriptor = socket(domain,type,protocol);
    sockaddr_in server_address;

    hostent *server_ptr = gethostbyname(hostname.c_str());
    if (!server_ptr) {
      cerr << "Unable to resolve hostname " << hostname << "\n";
      return;
    }

    memset(&server_address,0,sizeof server_address);
    server_address.sin_family = AF_INET;
    memcpy(
      &server_address.sin_addr.s_addr,server_ptr->h_addr,server_ptr->h_length
    );
    server_address.sin_port = htons(port);
    int connect_result =
      connect(
        data_socket_file_descriptor,
        reinterpret_cast<const sockaddr*>(&server_address),
        sizeof server_address
      );
    if (connect_result<0) {
      cerr << "Unable to connect to server\n";
      return;
    }
  }

  void closeConnection()
  {
    close(data_socket_file_descriptor);
    data_socket_file_descriptor = invalid_file_descriptor;
  }
};


static const int port = 4782;
static const int n_iterations = 10000;

static void runServer()
{
  Server server(port);

  for (int i=0; i!=n_iterations; ++i) {
    server.waitForConnection();
    server.waitForClientToClose();
    server.closeConnection();
    cerr << ".";
  }
}


static void runClient()
{
  Client client;

  for (int i=0; i!=n_iterations; ++i) {
    client.connectTo("localhost",port);
    client.closeConnection();
    cerr << ".";
  }
}


int main(int argc,char **argv)
{
  if (argc!=2) {
    cerr << "Usage: " << argv[0] << " (server|client)\n";
    return EXIT_FAILURE;
  }

  string operation = argv[1];

  if (operation=="server") {
    runServer();
  }
  else if (operation=="client") {
    runClient();
  }
}
