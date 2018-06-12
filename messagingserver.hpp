#include "objectset.hpp"
#include "socket.hpp"
#include "messagebuilder.hpp"
#include "messagesender.hpp"


class MessagingServer {
  public:
    using ClientHandle = ObjectSetHandle;
    using Message = std::string;
    using MessageHandler = std::function<void(ClientHandle,const Message &)>;
    using ConnectHandler = std::function<void(ClientHandle)>;
    using DisconnectHandler = std::function<void(ClientHandle)>;

    MessagingServer(int port);
    ~MessagingServer();

    void
      checkForEvents(
        const MessageHandler &,
        const ConnectHandler &,
        const DisconnectHandler &
      );

    int clientCount() const;
    void sendMessageToClient(const Message &message,ClientHandle);
    void forEachClient(const std::function<void(ClientHandle)> &);

  private:
    struct Client {
      Socket data_socket;
      MessageBuilder message_builder;
      MessageSender message_sender;

      void checkForMessages(const MessageHandler &,ClientHandle);
      void sendMessage(const Message &message);

      private:
        MessageSender::SendChunkFunction _sendChunkFunction();
    };

    void checkForNewClients(const ConnectHandler &);
    void checkForMessagesFromEachClient(const MessageHandler &);
    void removeDisconnectedClients(const DisconnectHandler &);
    bool aClientIsTryingToConnect() const;

    Socket listen_socket;
    ObjectSet<Client> clients;
};
