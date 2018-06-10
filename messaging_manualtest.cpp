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
using std::string;


static const int port = 4782;


static void
  sendFlood(
    MessagingServer &server,
    MessagingServer::ClientHandle client_handle,
    int count
  )
{
  for (int i=0; i!=count; ++i) {
    if (i%1000==0) {
      cerr << "Sending flood message " << i+1 << "\n";
    }
    server.sendMessageToClient("flood data",client_handle);
  }

  server.sendMessageToClient("end",client_handle);
}



static bool startsWith(const string &full_string,const string &prefix)
{
  return full_string.substr(0,prefix.length())==prefix;
}


static int floodCount(const string &message)
{
  std::istringstream stream(message);
  string first_word, second_word;
  stream >> first_word >> second_word;
  assert(first_word=="flood");
  return std::stoi(second_word);
}


static void runServer()
{
  MessagingServer server(port);
  bool quit_message_was_received = false;
  using ClientHandle = MessagingServer::ClientHandle;

  MessagingServer::MessageHandler message_handler =
    [&](ClientHandle client_handle,const string &message){
      assert(message.find('\0')==message.npos);
      cerr << "message from client " << client_handle << ": " <<
        message << "\n";

      for (int i=0, n=message.size(); i!=n; ++i) {
        assert(message[i]>0);
      }

      if (message=="quit") {
        quit_message_was_received = true;
      }
      else if (startsWith(message,"flood")) {
        int count = floodCount(message);
        sendFlood(server,client_handle,count);
      }
      else {
        server.sendMessageToClient("ack",client_handle);
      }
    };

  MessagingServer::ConnectHandler connect_handler =
    [](ClientHandle client_handle){
      cerr << "client " << client_handle << " connected.\n";
    };

  while (server.clientCount()!=0 || !quit_message_was_received) {
    server.checkForEvents(message_handler,connect_handler);
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


static string str(int i)
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

  auto message_handler = [&](const string &message){
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


static string floodMessage(int n)
{
  ostringstream stream;
  stream << "flood " << n;
  return stream.str();
}


static void runFloodClient()
{
  int n_messages_per_flood = 1000000;

  MessagingClient client;
  client.connectToServer("localhost",port);
  client.sendMessage(floodMessage(n_messages_per_flood));
  sleepForNSeconds(5);
  int n_data_messages_received = 0;
  bool finished = false;

  auto message_handler = [&](const string &message){
    if (message=="flood data") {
      ++n_data_messages_received;
      if (n_data_messages_received%1000 == 1) {
        cerr << "Got flood message " << n_data_messages_received << "\n";
      }
    }
    else if (message=="end") {
      finished = true;
    }
    else {
      cerr << "Unknown message: " << message << "\n";
      assert(false);
    }
  };

  while (!finished) {
    client.checkForMessages(message_handler);
  }

  assert(n_data_messages_received==n_messages_per_flood);
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
    cerr << "Usage: " << argv[0] << " <operation>\n";
    cerr << "\n";
    cerr << "Operations:\n";
    cerr << "  server\n";
    cerr << "  count_client\n";
    cerr << "  quit_client\n";
    cerr << "  flood_client\n";
    return EXIT_FAILURE;
  }

  string operation = argv[1];

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

  if (operation=="flood_client") {
    runFloodClient();
    return EXIT_SUCCESS;
  }

  cerr << "Unknown operation: " << operation << "\n";
  return EXIT_FAILURE;
}
