#include "messagesender.hpp"

#include <cassert>

using std::string;


const string &MessageSender::_remainingMessage() const
{
  assert(_a_message_is_being_sent);
  return _remaining_message;
}


void
  MessageSender::_sendBufferedMessages(
    const SendChunkFunction &send_chunk_function
  )
{
  while (!_buffered_messages.empty()) {
    size_t remaining_size =
      _flush(_buffered_messages[0],send_chunk_function);

    if (remaining_size!=0) {
      _storeRemaining(_buffered_messages[0],remaining_size);
      _buffered_messages.pop_front();
      return;
    }

    _buffered_messages.pop_front();
  }
}


void MessageSender::update(const SendChunkFunction &send_chunk_function)
{
  if (!_a_message_is_being_sent) {
    // The only reason we would have buffered messages is if we failed
    // to send a message, in which case _a_message_is_being_sent would
    // be true.
    assert(_buffered_messages.empty());
    return;
  }

  size_t remaining_size = _flush(_remainingMessage(),send_chunk_function);

  _storeRemaining(_remainingMessage(),remaining_size);

  if (!_a_message_is_being_sent) {
    _sendBufferedMessages(send_chunk_function);
  }
}



size_t
  MessageSender::_flush(
    const string &message,
    const SendChunkFunction &send_chunk_function
  )
{
  const char *chunk = message.c_str();
  size_t remaining_size = message.length() + 1;
    // message.c_str() gives us a null-terminated string.  Since we want
    // to send the null terminator, we'll just pretend like the message is
    // one byte longer.

  for (;;) {
    size_t chunk_size = remaining_size;

    if (chunk_size>_max_chunk_size) {
      chunk_size = _max_chunk_size;
    }

    size_t n_bytes_sent = send_chunk_function(chunk,chunk_size);

    remaining_size -= n_bytes_sent;
    chunk += n_bytes_sent;

    bool output_is_full = (n_bytes_sent!=chunk_size);

    if (output_is_full) {
      break;
    }

    assert(n_bytes_sent<=chunk_size);

    if (remaining_size==0) {
      break;
    }
  }

  return remaining_size;
}


void MessageSender::_storeRemaining(const string &message,size_t remaining_size)
{
  if (remaining_size==0) {
    _remaining_message.clear();
    _a_message_is_being_sent = false;
  }
  else {
    size_t n_bytes_sent = message.size() + 1 - remaining_size;
    _remaining_message =
      string(message.data() + n_bytes_sent,remaining_size - 1);
    _a_message_is_being_sent = true;
  }
}


void
  MessageSender::addMessage(
    const string &message,
    const SendChunkFunction &send_chunk_function
  )
{
  if (_a_message_is_being_sent) {
    update(send_chunk_function);
  }

  if (_a_message_is_being_sent) {
    _buffered_messages.push_back(message);
    return;
  }

  size_t remaining_size = _flush(message,send_chunk_function);

  _storeRemaining(message,remaining_size);
}
