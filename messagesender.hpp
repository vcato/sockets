#include <string>
#include <functional>
#include <vector>
#include <deque>


class MessageSender {
  public:
    using SendChunkFunction = std::function<size_t(const char *,size_t)>;

    void setMaxChunkSize(size_t arg) { _max_chunk_size = arg; }
    void addMessage(const std::string &,const SendChunkFunction &);
    void update(const SendChunkFunction &);
    bool messagesAreBuffered() const { return _a_message_is_being_sent; }

  private:
    size_t _max_chunk_size = 1024;
    std::string _remaining_message;
    bool _a_message_is_being_sent = false;
    std::deque<std::string> _buffered_messages;

    const std::string &_remainingMessage() const;
    size_t _flush(const std::string &message,const SendChunkFunction &);
    void _storeRemaining(const std::string &message,size_t remaining_size);
    void _sendBufferedMessages(const SendChunkFunction &send_chunk_function);
};
