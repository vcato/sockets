#include "feedmessagebuilder.hpp"


void
  feedMessageBuilder(
    MessageBuilder &message_builder,
    Socket &data_socket,
    const std::function<void(const std::string&)> &message_handler
  )
{
  while (data_socket.hasDataAvailableForReading()) {
    char chunk[1024];
    ssize_t n_bytes_read = data_socket.recv(chunk,sizeof chunk);
    if (n_bytes_read>0) {
      message_builder.addChunk(chunk,n_bytes_read,message_handler);
    }
    else if (n_bytes_read==0) {
      // Client closed gracefully
      data_socket.close();
      return;
    }
    else {
      // Client closed ungracefully
      data_socket.close();
      return;
    }
  }
}
