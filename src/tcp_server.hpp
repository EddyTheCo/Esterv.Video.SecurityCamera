#pragma once
#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include <unordered_map>

class Session : public std::enable_shared_from_this<Session> {
  explicit Session(boost::asio::ip::tcp::tcp::socket socket);
  void doWrite(const std::vector<uint8_t> &packet_data);
  void doBuff();
  void doRead();
  void execute();
  void stream();
  void start();
  bool is_writing_{false};
  boost::beast::websocket::stream<boost::asio::ip::tcp::tcp::socket>
      web_socket_;
  boost::beast::flat_buffer read_buffer_;
  std::vector<uint8_t> write_buf;
  uint8_t command_{0};
  uint32_t id_{0};
  uint32_t frame_index_{0};
  static uint32_t index_;
  static std::unordered_map<size_t, std::shared_ptr<Session>> sessions_;

public:
  static void sendFrame(const std::vector<uint8_t> &frame,
                        const uint8_t command = 1,
                        const uint32_t frame_index = 0);
  friend class Server;
};
class Server {

  void doAccept();
  boost::asio::ip::tcp::tcp::acceptor acceptor_;

public:
  Server(boost::asio::io_context &io_context, unsigned short port);
};
