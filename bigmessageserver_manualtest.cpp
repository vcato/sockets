#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include "messagingserver.hpp"

using std::cerr;
using std::string;
using std::ostringstream;
using RandomEngine = std::mt19937;


static int randomInt(int low,int high,RandomEngine &engine)
{
  return std::uniform_int_distribution<int>(low,high)(engine);
}


static string randomMessage(RandomEngine &engine,int length)
{
  string message;

  for (int i=0; i!=length; ++i) {
    message.push_back('a'+randomInt(0,25,engine));
  }

  return message;
}


static string paddedString(int value,int width)
{
  ostringstream stream;
  stream.width(width);
  stream << value;
  return stream.str();
}


int main(int argc,char** argv)
{
  if (argc != 3) {
    cerr << "Usage: bigmessageserver_manualtest <size> <port>\n";
    return EXIT_FAILURE;
  }

  int message_size = atoi(argv[1]);
  int port = atoi(argv[2]);

  if (message_size <= 0) {
    cerr << "Invalid size\n";
    return EXIT_FAILURE;
  }

  if (port <= 0) {
    cerr << "Invalid port\n";
  }

  MessagingServer server(port);
  using ClientHandle = MessagingServer::ClientHandle;
  RandomEngine engine(/*seed*/1);

  MessagingServer::MessageHandler message_handler =
    [](ClientHandle,const string &){};
  MessagingServer::ConnectHandler connect_handler =
    [&](ClientHandle client_handle){
      string size_string = paddedString(message_size,/*width*/7);
      string full_message = size_string + randomMessage(engine,message_size);
      server.sendMessageToClient(full_message,client_handle);
    };
  MessagingServer::DisconnectHandler disconnect_handler =
    [](ClientHandle){};

  for (;;) {
    server.checkForEvents(message_handler,connect_handler,disconnect_handler);
  }
}
