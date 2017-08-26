#ifndef INTERNETADDRESS_HPP_
#define INTERNETADDRESS_HPP_

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>


class InternetAddress {
  public:
    InternetAddress();
    ~InternetAddress();

    void setPort(int port);
    void setHostname(const std::string &hostname);
    socklen_t sockaddrSize() const;
    sockaddr *sockaddrPtr();
    const sockaddr *sockaddrPtr() const;

  private:
    sockaddr_in address;
};

#endif /* INTERNETADDRESS_HPP_ */
