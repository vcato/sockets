#include "messagingserver.hpp"

#include <iostream>
#include "feedmessagebuilder.hpp"

using std::cerr;


MessagingServer::MessagingServer(int port)
{
  int backlog = 1;

  listen_socket.bindTo(port);
  listen_socket.startListening(backlog);
}


MessagingServer::~MessagingServer()
{
  listen_socket.close();
}


bool MessagingServer::aClientIsTryingToConnect() const
{
  return listen_socket.canRecvWithoutBlocking();
}


void MessagingServer::checkForNewClients(const ConnectHandler &connect_handler)
{
  if (aClientIsTryingToConnect()) {
    ClientHandle client_handle = clients.allocate();
    Client &client = clients[client_handle];
    client.data_socket.acceptFrom(listen_socket);
    connect_handler(client_handle);
  }
}


void
  MessagingServer::checkForMessagesFromEachClient(
    const MessageHandler &message_handler
  )
{
  auto check_for_client_messages =
    [&](ClientHandle client_handle) {
      Client &client = clients[client_handle];
      client.checkForMessages(message_handler,client_handle);
    };

  clients.forEachHandle(check_for_client_messages);
}


void
  MessagingServer::removeDisconnectedClients(
    const DisconnectHandler &disconnect_handler
  )
{
  auto check_for_client_disconnect =
    [&](ClientHandle client_handle) {
      Client &client = clients[client_handle];

      if (!client.data_socket.isValid()) {
        disconnect_handler(client_handle);
        // Client closed the connection
        clients.deallocate(client_handle);
      }
    };

  clients.forEachHandle(check_for_client_disconnect);
}


void
  MessagingServer::checkForEvents(
    const MessageHandler& message_handler,
    const ConnectHandler &connect_handler,
    const DisconnectHandler &disconnect_handler
  )
{
  checkForNewClients(connect_handler);
  checkForMessagesFromEachClient(message_handler);
  removeDisconnectedClients(disconnect_handler);
}


MessageSender::SendChunkFunction MessagingServer::Client::_sendChunkFunction()
{
  return
    [this](const char *data,size_t data_size) -> size_t {
      if (!data_socket.canSendWithoutBlocking()) {
        return 0;
      }

      ssize_t send_result = data_socket.send(data,data_size);

      if (send_result<0) {
        return 0;
      }

      size_t n_bytes_sent = send_result;
      return n_bytes_sent;
    };
}


void MessagingServer::Client::sendMessage(const std::string &message)
{
  message_sender.addMessage(message,_sendChunkFunction());
}


int MessagingServer::clientCount() const
{
  return clients.allocatedCount();
}


void
  MessagingServer::Client::checkForMessages(
    const MessageHandler &message_handler,
    ClientHandle client_handle
  )
{
  assert(data_socket.isValid());

  std::function<void(const std::string&)> f =
    [&](const std::string &message){
      message_handler(client_handle,message);
    };

  feedMessageBuilder(message_builder,data_socket,f);

  message_sender.update(_sendChunkFunction());
}


void
  MessagingServer::sendMessageToClient(
    const std::string &message,
    ClientHandle client_handle
  )
{
  clients[client_handle].sendMessage(message);
}


void
  MessagingServer::forEachClient(
    const std::function<void(ClientHandle)> &function
  )
{
  clients.forEachHandle(function);
}
