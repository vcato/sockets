#include "messagesender.hpp"

#include <cassert>
#include <vector>
#include <iostream>

using std::vector;
using std::string;
using std::cerr;


static void testSendingSimpleMessage()
{
  MessageSender sender;
  sender.setMaxChunkSize(1024);
  vector<string> chunks;

  std::function<size_t(const char *,size_t)> send_chunk_function =
    [&](const char *chunk,size_t chunk_size) -> size_t
    {
      chunks.push_back(string(chunk,chunk_size));
      return chunk_size;
    };

  sender.addMessage("test",send_chunk_function);
  assert(chunks.size()==1);
}


static void testChunkSizeLessThanMessageSize()
{
  MessageSender sender;
  sender.setMaxChunkSize(1);
  vector<string> chunks;

  std::function<size_t(const char *,size_t)> send_chunk_function =
    [&](const char *chunk,size_t chunk_size) -> size_t
    {
      assert(chunk_size==1);
      chunks.push_back(string(chunk,chunk_size));
      return chunk_size;
    };

  sender.addMessage("test",send_chunk_function);

  assert(chunks.size()==5);
  assert(chunks[0]=="t");
  assert(chunks[1]=="e");
  assert(chunks[2]=="s");
  assert(chunks[3]=="t");
  assert(chunks[4]==string(1,'\0'));
}


static void testPartialSending()
{
  MessageSender sender;
  size_t max_chunk_size = 2;
  sender.setMaxChunkSize(max_chunk_size);
  vector<string> chunks;

  std::function<size_t(const char *,size_t)> send_chunk_function =
    [&](const char *chunk,size_t chunk_size) -> size_t
    {
      assert(chunk_size<=max_chunk_size);

      if (!chunks.empty()) {
        return 0;
      }

      chunks.push_back(string(chunk,chunk_size));
      return chunk_size;
    };

  sender.addMessage("test",send_chunk_function);

  assert(chunks.size()==1);
  assert(chunks[0]=="te");

  chunks.clear();

  sender.update(send_chunk_function);

  assert(chunks.size()==1);
  assert(chunks[0]=="st");

  chunks.clear();

  sender.update(send_chunk_function);

  assert(chunks.size()==1);
  assert(chunks[0].length()==1);
  assert(chunks[0]==string(1,'\0'));

  chunks.clear();

  sender.update(send_chunk_function);

  assert(chunks.empty());
}


static void testAddingAMessageWhenOutputIsPending()
{
  MessageSender sender;
  size_t max_chunk_size = 2;
  sender.setMaxChunkSize(max_chunk_size);
  vector<string> chunks;

  std::function<size_t(const char *,size_t)> send_chunk_function =
    [&](const char *chunk,size_t chunk_size) -> size_t
    {
      assert(chunk_size<=max_chunk_size);

      if (!chunks.empty()) {
        return 0;
      }

      chunks.push_back(string(chunk,chunk_size));
      return chunk_size;
    };

  sender.addMessage("ab",send_chunk_function);

  assert(operator==(chunks,{"ab"}));

  chunks.clear();

  sender.addMessage("another",send_chunk_function);

  assert(operator==(chunks,{string("\0",1)}));

  chunks.clear();

  sender.update(send_chunk_function);

  assert(operator==(chunks,{"an"}));
}


static void testAddingAMessageWhenOutputIsPending2()
{
  MessageSender sender;
  size_t max_chunk_size = 2;
  sender.setMaxChunkSize(max_chunk_size);
  vector<string> chunks;

  std::function<size_t(const char *,size_t)> send_chunk_function =
    [&](const char *chunk,size_t chunk_size) -> size_t
    {
      assert(chunk_size<=max_chunk_size);

      if (!chunks.empty()) {
        return 0;
      }

      chunks.push_back(string(chunk,chunk_size));
      return chunk_size;
    };

  sender.addMessage("test",send_chunk_function);
  assert(operator==(chunks,{"te"}));

  chunks.clear();
  sender.addMessage("another",send_chunk_function);
  assert(operator==(chunks,{"st"}));

  chunks.clear();
  sender.update(send_chunk_function);
  assert(operator==(chunks,{string("\0",1)}));

  chunks.clear();
  sender.update(send_chunk_function);
  assert(operator==(chunks,{"an"}));
}


static void testMultipleBufferedMessages()
{
  MessageSender sender;
  size_t max_chunk_size = 2;
  sender.setMaxChunkSize(max_chunk_size);
  vector<string> chunks;

  std::function<size_t(const char *,size_t)> send_chunk_function =
    [&](const char *chunk,size_t chunk_size) -> size_t
    {
      assert(chunk_size<=max_chunk_size);

      if (!chunks.empty()) {
        return 0;
      }

      chunks.push_back(string(chunk,chunk_size));
      return chunk_size;
    };

  sender.addMessage("test",send_chunk_function);
  sender.addMessage("x",send_chunk_function);
  sender.addMessage("y",send_chunk_function);
  assert(operator==(chunks,{"te"}));

  chunks.clear();
  sender.update(send_chunk_function);
  assert(operator==(chunks,{"st"}));

  chunks.clear();
  sender.update(send_chunk_function);
  assert(operator==(chunks,{string("\0",1)}));

  chunks.clear();
  sender.update(send_chunk_function);
  assert(operator==(chunks,{string("x\0",2)}));

  chunks.clear();
  sender.update(send_chunk_function);
  assert(operator==(chunks,{string("y\0",2)}));

  chunks.clear();
  sender.update(send_chunk_function);
  assert(chunks.empty());
}


int main()
{
  testSendingSimpleMessage();
  testChunkSizeLessThanMessageSize();
  testPartialSending();
  testAddingAMessageWhenOutputIsPending();
  testAddingAMessageWhenOutputIsPending2();
  testMultipleBufferedMessages();
}
