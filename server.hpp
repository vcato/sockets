#include "socket.hpp"


class Server {
  public:
    Server(int port);
    ~Server();

    void waitForConnection();
    void waitForClientToClose();
    void closeConnection();

  private:
    Socket listen_socket;
    Socket data_socket;

    void startListening(int port);
    void stopListening();
    void acceptConnection();
};
