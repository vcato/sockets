#include <iostream>
#include <thread>
#include <sstream>
#include <random>
#include <map>
#include <deque>
#include "messagingserver.hpp"
#include "messagingclient.hpp"
#ifndef _WIN32
#include <signal.h>
#endif


using std::cerr;
using std::ostringstream;
using std::string;
using std::map;
using std::make_unique;
using RandomEngine = std::mt19937;


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


static int echoCountOf(const string &message)
{
  std::istringstream stream(message);
  string first_word, second_word;
  stream >> first_word >> second_word;
  assert(first_word=="echo");
  return std::stoi(second_word);
}


static int sendSizeOf(const string &message)
{
  std::istringstream stream(message);
  string first_word, second_word;
  stream >> first_word >> second_word;
  assert(first_word=="sendme");
  return std::stoi(second_word);
}


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


static string randomMessage(RandomEngine &engine)
{
  int length = randomInt(0,10000,engine);
  return randomMessage(engine,length);
}


static void runServer()
{
  MessagingServer server(port);
  bool quit_message_was_received = false;
  using ClientHandle = MessagingServer::ClientHandle;
  map<ClientHandle,int> echo_counts;
  RandomEngine engine(/*seed*/1);

  MessagingServer::MessageHandler message_handler =
    [&](ClientHandle client_handle,const string &message){
      assert(message.find('\0')==message.npos);

      for (int i=0, n=message.size(); i!=n; ++i) {
        assert(message[i]>0);
      }

      if (echo_counts.count(client_handle)) {
        server.sendMessageToClient(message,client_handle);
        --echo_counts[client_handle];
        if (echo_counts[client_handle]==0) {
          echo_counts.erase(client_handle);
        }
      }
      else if (message=="quit") {
        quit_message_was_received = true;
      }
      else if (startsWith(message,"flood")) {
        int count = floodCount(message);
        sendFlood(server,client_handle,count);
      }
      else if (startsWith(message,"echo")) {
        int count = echoCountOf(message);
        echo_counts[client_handle] += count;
      }
      else if (startsWith(message,"tellall")) {
        server.forEachClient(
          [&](ClientHandle client_handle){
            server.sendMessageToClient("hi everyone",client_handle);
          }
        );
      }
      else if (startsWith(message,"sendme ")) {
        int size_to_send = sendSizeOf(message);
        string message = randomMessage(engine,size_to_send);
        server.sendMessageToClient(message,client_handle);
      }
      else {
        server.sendMessageToClient("ack",client_handle);
      }
    };

  auto connect_handler =
    [](ClientHandle client_handle){
      cerr << "client " << client_handle << " connected.\n";
    };

  auto disconnect_handler =
    [](ClientHandle client_handle){
      cerr << "client " << client_handle << " disconnected.\n";
    };

  while (server.clientCount()!=0 || !quit_message_was_received) {
    server.checkForEvents(message_handler,connect_handler,disconnect_handler);
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
    if (!client.isConnected()) {
      cerr << "Server disconnected\n";
      assert(false);
      return;
    }
    client.checkForMessages(message_handler);
  }

  assert(n_data_messages_received==n_messages_per_flood);
  client.disconnectFromServer();
}


static void runRandomClient()
{
  MessagingClient client;
  client.connectToServer("localhost",port);
  int echo_count = 10000;
  client.sendMessage("echo " + str(echo_count));
  int n_messages_received = 0;
  RandomEngine engine(/*seed*/1);
  std::deque<string> expected_messages;

  auto message_handler = [&](const string &message){
    assert(message==expected_messages.front());
    expected_messages.pop_front();

    ++n_messages_received;
  };

  for (int i=0; i!=echo_count; ++i) {
    string message = randomMessage(engine);
    client.sendMessage(message);
    expected_messages.push_back(message);
    client.checkForMessages(message_handler);
  }

  while (n_messages_received!=echo_count) {
    client.checkForMessages(message_handler);
  }

  client.disconnectFromServer();
}


static void runTellAllClient()
{
  MessagingClient client;
  client.connectToServer("localhost",port);
  client.sendMessage("tellall");
  client.disconnectFromServer();
}


static void runBigReceiveClient()
{
  MessagingClient client;
  client.connectToServer("localhost",port);
  client.sendMessage("sendme 1000000");
  std::unique_ptr<string> received_message_ptr;

  auto message_handler =
    [&](const string &message){
      received_message_ptr = make_unique<string>(message);
    };

  for (;;) {
    client.checkForMessages(message_handler);

    if (received_message_ptr) {
      break;
    }
  }

  assert(received_message_ptr->size() == 1000000);

  client.disconnectFromServer();
}


static void runListenClient()
{
  MessagingClient client;
  client.connectToServer("localhost",port);
  bool got_a_message = false;

  while (!got_a_message) {
    client.checkForMessages([&](const string &message){
      cerr << "Got message: " << message << "\n";
      got_a_message = true;
    });
  }

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
    cerr << "  random_client\n";
    cerr << "  tellall_client\n";
    cerr << "  listen_client\n";
    return EXIT_FAILURE;
  }

  string operation = argv[1];

  if      (operation=="server")            runServer();
  else if (operation=="count_client")      runCountClient();
  else if (operation=="quit_client")       runQuitClient();
  else if (operation=="flood_client")      runFloodClient();
  else if (operation=="random_client")     runRandomClient();
  else if (operation=="tellall_client")    runTellAllClient();
  else if (operation=="bigreceive_client") runBigReceiveClient();
  else if (operation=="listen_client")     runListenClient();
  else {
    cerr << "Unknown operation: " << operation << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
