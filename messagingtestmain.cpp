#include <iostream>
#include <thread>
#include <sstream>
#include "messagingserver.hpp"
#include "messagingclient.hpp"
#ifndef _WIN32
#include <signal.h>
#endif


using std::cerr;
using std::ostringstream;


static const int port = 4782;


static void runServer()
{
  MessagingServer server(port);
  bool quit_message_was_received = false;
  using ClientHandle = MessagingServer::ClientHandle;

  MessagingServer::MessageHandler message_handler =
    [&](ClientHandle client_handle,const std::string &message){
      assert(message.find('\0')==message.npos);
      cerr << "message from client " << client_handle << ": " <<
        message << "\n";

      for (int i=0, n=message.size(); i!=n; ++i) {
        assert(message[i]>0);
      }

      if (message=="quit") {
        quit_message_was_received = true;
      }
      else {
        server.sendMessageToClient("ack",client_handle);
      }
    };

  while (server.clientCount()!=0 || !quit_message_was_received) {
    server.checkForMessages(message_handler);
  }
}


static void sleepForNSeconds(float n)
{
  int n_milliseconds = n*1000;
#ifdef _WIN32
  Sleep(n_milliseconds);
#else
  std::this_thread::sleep_for(std::chrono::milliseconds(n_milliseconds));
#endif
}


static std::string str(int i)
{
  ostringstream stream;
  stream << i;
  return stream.str();
}


static void runCountClient()
{
  MessagingClient client;

  client.connectToServer("localhost",port);
  int n_acks_received = 0;

  auto message_handler = [&](const std::string &message){
    if (message=="ack") {
      ++n_acks_received;
    }
    else {
      cerr << "Got message: " << message << "\n";
    }
  };

  for (int i=1; i<=5; ++i) {
    if (!client.isConnected()) {
      cerr << "Server closed connection\n";
      return;
    }

    client.sendMessage(str(i));
    sleepForNSeconds(1);
    client.checkForMessages(message_handler);
    cerr << "n_acks_received=" << n_acks_received << "\n";
  }

  client.disconnectFromServer();
}


static void runQuitClient()
{
  MessagingClient client;

  client.connectToServer("localhost",port);
  client.sendMessage("quit");
  client.disconnectFromServer();
}


#ifndef _WIN32
static void disablePipeSignal()
{
  signal(SIGPIPE,SIG_IGN);
}
#endif


int main(int argc,char** argv)
{
  Socket::initialize();

#ifndef _WIN32
  // The pipe signal needs to be disabled so we can treat unexpected
  // connection loss as a read/write error.
  disablePipeSignal();
#endif

  if (argc!=2) {
    cerr << "Usage: " << argv[0] << " (server|count_client|quit_client)\n";
    return EXIT_FAILURE;
  }

  std::string operation = argv[1];

  if (operation=="server") {
    runServer();
    return EXIT_SUCCESS;
  }

  if (operation=="count_client") {
    runCountClient();
    return EXIT_SUCCESS;
  }

  if (operation=="quit_client") {
    runQuitClient();
    return EXIT_SUCCESS;
  }

  cerr << "Unknown operation: " << operation << "\n";
  return EXIT_FAILURE;
}
