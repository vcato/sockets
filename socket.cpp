#include "socket.hpp"

#ifdef WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#endif
#include <stdexcept>
#include <cassert>


Socket::Socket()
  : socket_handle(invalid_socket_handle)
{
}


Socket::Socket(Socket&& arg) noexcept
: socket_handle(arg.socket_handle)
{
  std::swap(socket_handle,arg.socket_handle);
}


Socket::Socket(SocketHandle socket_handle_arg)
: socket_handle(socket_handle_arg)
{
}


Socket::~Socket()
{
  assert(socket_handle==invalid_socket_handle);
}


Socket& Socket::operator=(Socket&& arg)
{
  assert(socket_handle==invalid_socket_handle);
  std::swap(socket_handle,arg.socket_handle);
  return *this;
}


void Socket::bind(const Socket &socket,const InternetAddress &address)
{
  SocketHandle sockfd = socket.socket_handle;
  const sockaddr *addr = address.sockaddrPtr();
  socklen_t addrlen = address.sockaddrSize();

  int bind_result = ::bind(sockfd,addr,addrlen);

  if (bind_result==socket_error) {
    throw std::runtime_error("Unable to bind socket.");
  }
}


Socket Socket::accept(const Socket &listen_socket)
{
  InternetAddress client_address;
  SocketHandle sockfd = listen_socket.socket_handle;
  sockaddr *addr = client_address.sockaddrPtr();
  socklen_t addrlen = client_address.sockaddrSize();
  SocketHandle new_file_descriptor = ::accept(sockfd,addr,&addrlen);

  if (new_file_descriptor==invalid_socket_handle) {
    throw std::runtime_error("Failed to accept connection.");
  }

  return Socket(new_file_descriptor);
}


void
  Socket::connect(
    const Socket &data_socket,
    const InternetAddress &server_address
  )
{
  SocketHandle sockfd = data_socket.socket_handle;
  const sockaddr *addr = server_address.sockaddrPtr();
  socklen_t addrlen = server_address.sockaddrSize();
  int connect_result = ::connect(sockfd,addr,addrlen);

  if (connect_result==socket_error) {
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
  int listen_result = ::listen(socket_handle,backlog);

  if (listen_result==socket_error) {
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
  SocketHandle new_file_descriptor = socket(domain,type,protocol);

  if (new_file_descriptor==invalid_socket_handle) {
    throw std::runtime_error("Failed to create socket.");
  }

  assert(socket_handle==invalid_socket_handle);
  socket_handle = new_file_descriptor;
}


void Socket::connectTo(const std::string &hostname,int port)
{
  InternetAddress server_address;

  server_address.setHostname(hostname);
  server_address.setPort(port);

  create();
  connect(*this,server_address);
}


void Socket::close()
{
#ifdef _WIN32
  closesocket(socket_handle);
#else
  ::close(socket_handle);
#endif
  socket_handle = invalid_socket_handle;
}


ssize_t Socket::recv(void *buffer,size_t n_bytes) const
{
  assert(isValid());

  int flags = 0;
  ssize_t n_read =
    ::recv(socket_handle,static_cast<char*>(buffer),n_bytes,flags);
  return n_read;
}


ssize_t Socket::send(const void *buffer,size_t n_bytes) const
{
  assert(isValid());

  int flags = 0;
  ssize_t n_written =
    ::send(socket_handle,static_cast<const char*>(buffer),n_bytes,flags);

  return n_written;
}


void Socket::initialize()
{
#ifdef _WIN32
  static WSADATA wsa;

  if (WSAStartup(MAKEWORD(2,2),&wsa)!=0) {
    throw std::runtime_error("Unable to initialize winsock.");
  }
#endif
}


bool Socket::hasDataAvailableForReading() const
{
  assert(socket_handle!=invalid_socket_handle);

  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(socket_handle,&read_set);
  int nfds = socket_handle+1;
  timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  select(nfds,&read_set,/*writefds*/0,/*exceptfds*/0,&timeout);

  if (FD_ISSET(socket_handle,&read_set)) {
    return true;
  }

  return false;
}


bool Socket::isValid() const
{
  return socket_handle!=invalid_socket_handle;
}
