#include "messagebuilder.hpp"

#include <iostream>

using std::cerr;


void
  MessageBuilder::addChunk(
    const char *chunk,
    size_t chunk_size,
    const MessageHandler& message_handler
  )
{
  size_t i=0;

  while (i!=chunk_size) {
    size_t begin = i;

    while (i!=chunk_size && chunk[i]!='\0') {
      ++i;
    }

    message_so_far.append(chunk+begin,i-begin);

    if (i==chunk_size) {
      // Got a partial message_so_far
      return;
    }

    assert(chunk[i]=='\0');
    message_handler(message_so_far);
    message_so_far.clear();

    ++i;
  }
}

