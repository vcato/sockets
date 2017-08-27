#include "messagebuilder.hpp"

#include <iostream>
#include <cassert>
#include <vector>


using std::cerr;


static void test1()
{
  MessageBuilder builder;
  char chunk[] = "test";
  bool got_message = false;
  MessageBuilder::MessageHandler handler =
    [&](const std::string &message){
      assert(!got_message);
      assert(message=="test");
      got_message = true;
    };

  builder.addChunk(chunk,sizeof chunk,handler);
  assert(got_message);
}


static void test2()
{
  MessageBuilder builder;
  bool got_message = false;

  auto handler = [&](const std::string &message){
    assert(!got_message);
    assert(message=="test");
    got_message = true;
  };

  {
    char chunk[] = {'t','e','s','t'};
    builder.addChunk(chunk,sizeof chunk,handler);
  }
  {
    char chunk[] = {'\0'};
    builder.addChunk(chunk,sizeof chunk,handler);
  }
}


static void test3()
{
  MessageBuilder builder;
  std::vector<std::string> received_messages;
  auto handler = [&](const std::string &message){
    received_messages.push_back(message);
  };

  char chunk[] = "test1\0test2";
  builder.addChunk(chunk,sizeof chunk,handler);

  assert(received_messages.size()==2);
  assert(received_messages[0]=="test1");
  assert(received_messages[1]=="test2");
}


int main()
{
  test1();
  test2();
  test3();
}
