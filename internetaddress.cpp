#include "internetaddress.hpp"

#ifdef _WIN32
#else
#include <netdb.h>
#endif
#include <sstream>
#include <cstring>


InternetAddress::InternetAddress()
{
  memset(&address,0,sizeof address);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
}


InternetAddress::~InternetAddress()
{
}


void InternetAddress::setPort(int port)
{
  address.sin_port = htons(port);
}


void InternetAddress::setHostname(const std::string &hostname)
{
  hostent *server_ptr = gethostbyname(hostname.c_str());

  if (!server_ptr) {
    std::ostringstream stream;
    stream << "Unable to resolve hostname " << hostname << ".";
    throw std::runtime_error(stream.str());
  }

  memcpy(&address.sin_addr.s_addr,server_ptr->h_addr,server_ptr->h_length);
}


socklen_t InternetAddress::sockaddrSize() const
{
  return sizeof address;
}


sockaddr *InternetAddress::sockaddrPtr()
{
  return reinterpret_cast<sockaddr*>(&address);
}


const sockaddr *InternetAddress::sockaddrPtr() const
{
  return reinterpret_cast<const sockaddr*>(&address);
}
