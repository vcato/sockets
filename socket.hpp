#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include "internetaddress.hpp"


class Socket {
  public:
    Socket();
    Socket(Socket&& arg) noexcept;
    ~Socket();

    Socket& operator=(Socket&& arg);

    bool isValid() const;
    bool hasDataAvailableForReading() const;
    void bindTo(int port);
    void startListening(int backlog);
    void acceptFrom(const Socket &listen_socket);
    void connectTo(const std::string &hostname,int port);
    void close();
    ssize_t recv(char *buffer,size_t n_bytes) const;
    ssize_t send(const char *buffer,size_t n_bytes) const;
    static void initialize();

  private:
#ifdef _WIN32
    using SocketHandle = SOCKET;
    static const SocketHandle invalid_socket_handle = INVALID_SOCKET;
    static const int socket_error = SOCKET_ERROR;
#else
    using SocketHandle = int;
    static const SocketHandle invalid_socket_handle = -1;
    static const int socket_error = -1;
#endif
    SocketHandle socket_handle;

    Socket(SocketHandle);
    void create();
    static void bind(const Socket &,const InternetAddress &);
    static Socket accept(const Socket &listen_socket);
    static void connect(const Socket &,const InternetAddress &);
};

#endif /* SOCKET_HPP_ */
