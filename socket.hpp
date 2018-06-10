#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include "internetaddress.hpp"


class Socket {
  public:
    Socket();
    Socket(Socket&& arg) noexcept;
    Socket(const Socket&) = delete;
    ~Socket();

    struct NonBlockingRecvResult {
      bool would_block;
      ssize_t n_bytes_received;
    };

    Socket& operator=(Socket&& arg);
    void operator=(const Socket& arg) = delete;

    static void initialize();

    bool isValid() const;
    void bindTo(int port);
    void startListening(int backlog);

    void setNoDelay(bool);
    void setNonBlocking(bool non_blocking);

    bool canRecvWithoutBlocking() const;
    bool canSendWithoutBlocking() const;

    void acceptFrom(const Socket &listen_socket);
    void connectTo(const std::string &hostname,int port);
    void close();

    ssize_t recv(char *buffer,size_t n_bytes) const;
      // Returns -1 on error, 0 if the remote end closed, or a positive value
      // indicating the number of bytes received.

    NonBlockingRecvResult nonBlockingRecv(char *buffer,size_t n_bytes) const;
      // Returns would_block==true if the receive failed because the socket
      // was non-blocking and no data was available.  Otherwise, sets
      // n_bytes_received to -1 on error, 0 if the remote end closed, or
      // a positive value indicating the number of bytes received.

    ssize_t send(const char *buffer,size_t n_bytes) const;

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
