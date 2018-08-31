#include <iostream>
#include <string>
#include <cassert>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include "internetaddress.hpp"


using std::cerr;
using std::string;

const int port = 7181;
using SocketHandle = int;
static const SocketHandle invalid_socket_handle = -1;


static void runServer()
{
  int domain = AF_INET;
  int type = SOCK_DGRAM;
  int protocol = 0;
  SocketHandle new_file_descriptor = socket(domain,type,protocol);

  if (new_file_descriptor==invalid_socket_handle) {
    throw std::runtime_error("Failed to create socket.");
  }

  SocketHandle socket_handle = new_file_descriptor;

  InternetAddress address;
  address.setPort(port);
  const sockaddr *addr = address.sockaddrPtr();
  socklen_t addrlen = address.sockaddrSize();
  int bind_result = ::bind(socket_handle,addr,addrlen);

  if (bind_result==-1) {
    throw std::runtime_error("Failed to bind socket.");
  }

  char buffer[1024];

  cerr << "Waiting...";
  ssize_t recv_result = recv(socket_handle,buffer,sizeof buffer,/*flags*/0);

  cerr << "recv_result: " << recv_result << '\n';
}


static void runClient()
{
  int domain = AF_INET;
  int type = SOCK_DGRAM;
  int protocol = 0;
  SocketHandle new_file_descriptor = socket(domain,type,protocol);

  if (new_file_descriptor==invalid_socket_handle) {
    throw std::runtime_error("Failed to create socket.");
  }

  SocketHandle socket_handle = new_file_descriptor;
  InternetAddress address;
  address.setPort(port);
  address.setHostname("localhost");

  const size_t bufsize = 32768;
  char buffer[bufsize] = "test";
  const sockaddr *addr = address.sockaddrPtr();
  socklen_t addrlen = address.sockaddrSize();
  ssize_t send_result =
    sendto(
      socket_handle,buffer,bufsize,/*flags*/0,
      addr,addrlen
    );

  cerr << "send_result: " << send_result << '\n';

  if (send_result==-1) {
    perror("send");
  }
}


int main(int argc,char** argv)
{
  if (argc==1) {
    cerr << "Usage: udp_manualtest (server|client <hostname>)\n";
    return EXIT_FAILURE;
  }

  string operation = argv[1];

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
