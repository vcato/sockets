#include "objectset.hpp"
#include "socket.hpp"
#include "messagebuilder.hpp"


class MessagingServer {
  public:
    using ClientHandle = ObjectSetHandle;
    using Message = std::string;
    using MessageHandler = std::function<void(ClientHandle,const Message &)>;
    using ConnectHandler = std::function<void(ClientHandle)>;

    MessagingServer(int port);
    ~MessagingServer();

    void checkForEvents(const MessageHandler &,const ConnectHandler &);
    int clientCount() const;
    void sendMessageToClient(const Message &message,ClientHandle);

  private:
    struct Client {
      Socket data_socket;
      MessageBuilder message_builder;

      void checkForMessages(const MessageHandler &,ClientHandle);
      void sendMessage(const Message &message);
    };

    void checkForNewClients(const ConnectHandler &);
    void checkForMessagesFromEachClient(const MessageHandler &);
    void removeDisconnectedClients();
    bool aClientIsTryingToConnect() const;

    Socket listen_socket;
    ObjectSet<Client> clients;
};
