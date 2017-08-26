#include <string>
#include "socket.hpp"


class Client {
  public:
    void connectTo(const std::string &hostname,int port);
    void closeConnection();

  private:
    Socket data_socket;
};
