#include "socket.hpp"

#include <stdexcept>
#include <cassert>
#include <unistd.h>


Socket::Socket()
  : file_descriptor(invalid_file_descriptor)
{
}


Socket::Socket(Socket&& arg)
: file_descriptor(arg.file_descriptor)
{
  std::swap(file_descriptor,arg.file_descriptor);
}


Socket::~Socket()
{
  assert(file_descriptor==invalid_file_descriptor);
}


Socket& Socket::operator=(Socket&& arg)
{
  assert(file_descriptor==invalid_file_descriptor);
  std::swap(file_descriptor,arg.file_descriptor);
  return *this;
}


void Socket::bind(const Socket &socket,const InternetAddress &address)
{
  int sockfd = socket.file_descriptor;
  const sockaddr *addr = address.sockaddrPtr();
  socklen_t addrlen = address.sockaddrSize();

  int bind_result = ::bind(sockfd,addr,addrlen);

  if (bind_result!=0) {
    throw std::runtime_error("Unable to bind socket.");
  }
}


Socket Socket::accept(const Socket &listen_socket)
{
  Socket data_socket;
  InternetAddress client_address;
  int sockfd = listen_socket.file_descriptor;
  sockaddr *addr = client_address.sockaddrPtr();
  socklen_t addrlen = client_address.sockaddrSize();

  int new_file_descriptor = ::accept(sockfd,addr,&addrlen);

  if (new_file_descriptor==-1) {
    throw std::runtime_error("Failed to accept connection.");
  }

  assert(data_socket.file_descriptor==invalid_file_descriptor);
  data_socket.file_descriptor = new_file_descriptor;
  return data_socket;
}


void
  Socket::connect(
    const Socket &data_socket,
    const InternetAddress &server_address
  )
{
  int sockfd = data_socket.file_descriptor;
  const sockaddr *addr = server_address.sockaddrPtr();
  socklen_t addrlen = server_address.sockaddrSize();

  int connect_result = ::connect(sockfd,addr,addrlen);

  if (connect_result<0) {
    throw std::runtime_error("Unable to connect to server.");
  }
}


void Socket::bindTo(int port)
{
  InternetAddress server_address;
  server_address.setPort(port);

  create();
  bind(*this,server_address);
}


void Socket::startListening(int backlog)
{
  int listen_result = ::listen(file_descriptor,backlog);

  if (listen_result!=0) {
    throw std::runtime_error("Unable to listen.");
  }
}


void Socket::acceptFrom(const Socket &listen_socket)
{
  *this = accept(listen_socket);
}


void Socket::create()
{
  int domain = AF_INET;
  int type = SOCK_STREAM;
  int protocol = 0;
  int new_file_descriptor = socket(domain,type,protocol);

  if (new_file_descriptor==-1) {
    throw std::runtime_error("Failed to create socket.");
  }

  assert(file_descriptor==invalid_file_descriptor);
  file_descriptor = new_file_descriptor;
}


void Socket::connectTo(const InternetAddress &server_address)
{
  connect(*this,server_address);
}


void Socket::connectTo(const std::string &hostname,int port)
{
  InternetAddress server_address;

  server_address.setHostname(hostname);
  server_address.setPort(port);

  create();
  connectTo(server_address);
}


void Socket::close()
{
  ::close(file_descriptor);
  file_descriptor = invalid_file_descriptor;
}


ssize_t Socket::recv(void *buffer,size_t n_bytes) const
{
  assert(file_descriptor!=invalid_file_descriptor);
  ssize_t n_read = ::read(file_descriptor,buffer,n_bytes);
  return n_read;
}
