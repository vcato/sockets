#include "messagebuilder.hpp"
#include "socket.hpp"


extern void
  feedMessageBuilder(
    MessageBuilder &message_builder,
    Socket &data_socket,
    const std::function<void(const std::string&)> &message_handler
  );
