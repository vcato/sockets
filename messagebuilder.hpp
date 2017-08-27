#ifndef MESSAGEBUILDER_HPP_
#define MESSAGEBUILDER_HPP_

#include <string>
#include <functional>
#include <cassert>

struct MessageBuilder {
  std::string message_so_far;
  using MessageHandler = std::function<void(const std::string &)>;

  void
    addChunk(
      const char *chunk,
      size_t chunk_size,
      const MessageHandler& message_handler
    );
};

#endif /* MESSAGEBUILDER_HPP_ */
