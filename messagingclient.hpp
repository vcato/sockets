#include <string>
#include <functional>
#include "socket.hpp"
#include "messagebuilder.hpp"


class MessagingClient {
  public:
    void connectToServer(const std::string &hostname,int port);
    void disconnectFromServer();
    void sendMessage(const std::string &message);
    void checkForMessages(const std::function<void(std::string)> &f);
    bool isConnected() const;

  private:
    Socket data_socket;
    MessageBuilder message_builder;
};
