#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include "internetaddress.hpp"


class Socket {
  public:
    Socket();
    Socket(Socket&& arg);
    ~Socket();

    Socket& operator=(Socket&& arg);

    void bindTo(int port);
    void startListening(int backlog);
    void acceptFrom(const Socket &listen_socket);
    void connectTo(const std::string &hostname,int port);
    void close();
    ssize_t recv(void *buffer,size_t n_bytes) const;

  private:
    static const int invalid_file_descriptor = -1;
    int file_descriptor;

    void create();
    void connectTo(const InternetAddress &server_address);
    static void bind(const Socket &,const InternetAddress &);
    static Socket accept(const Socket &listen_socket);
    static void connect(const Socket &,const InternetAddress &);
};

#endif /* SOCKET_HPP_ */
