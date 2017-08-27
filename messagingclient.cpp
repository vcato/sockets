#include "messagingclient.hpp"

#include "feedmessagebuilder.hpp"


void MessagingClient::connectToServer(const std::string &hostname,int port)
{
  data_socket.connectTo(hostname,port);
}


void MessagingClient::disconnectFromServer()
{
  data_socket.close();
}


void MessagingClient::sendMessage(const std::string &message)
{
  data_socket.send(message.c_str(),message.size()+1);
}


void
  MessagingClient::checkForMessages(const std::function<void(std::string)> &f)
{  
  assert(data_socket.isValid());
  feedMessageBuilder(message_builder,data_socket,f);
}


bool MessagingClient::isConnected() const
{
  return data_socket.isValid();
}
