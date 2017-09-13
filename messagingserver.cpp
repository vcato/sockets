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


void MessagingServer::checkForNewClients()
{
  if (listen_socket.hasDataAvailableForReading()) {
    ClientHandle client_handle = clients.allocate();
    Client &client = clients[client_handle];
    client.data_socket.acceptFrom(listen_socket);
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

      if (!client.data_socket.isValid()) {
        // Client closed the connection
        clients.deallocate(client_handle);
      }
    };

  clients.forEachHandle(check_for_client_messages);
}


void MessagingServer::checkForMessages(const MessageHandler& message_handler)
{
  checkForNewClients();
  checkForMessagesFromEachClient(message_handler);
}


void MessagingServer::Client::sendMessage(const std::string &message)
{
  data_socket.send(message.c_str(),message.size()+1);
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
}


void
  MessagingServer::sendMessageToClient(
    const std::string &message,
    ClientHandle client_handle
  )
{
  clients[client_handle].sendMessage(message);
}
