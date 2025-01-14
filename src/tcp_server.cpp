#include "tcp_server.hpp"
#include "camera_service.hpp"
#include <iostream>
using boost::asio::ip::tcp;

std::unordered_map<size_t, std::shared_ptr<Session>> Session::sessions_;
uint32_t Session::index_{0};

Session::Session(boost::asio::ip::tcp::tcp::socket socket)
    : web_socket_(std::move(socket)) {
  web_socket_.binary(true);
}

void Session::start() {
  web_socket_.async_accept(
      [self = shared_from_this()](boost::system::error_code ec) {
        if (!ec)
          self->doRead();
      });
}
void Session::doRead() {
  auto self(shared_from_this());
  sessions_.insert({index_, self});
  id_ = index_;
  index_++;
  web_socket_.async_read(read_buffer_, [self](boost::system::error_code ec,
                                              std::size_t length) {
    if (!ec) {
      self->execute();
      self->doRead();
    } else {
      std::cerr << ec.message() << std::endl;
      self->web_socket_.async_close(boost::beast::websocket::close_code::normal,
                                    [](boost::beast::error_code ec) {});
      sessions_.extract(self->id_);
    }
  });
}

void Session::sendFrame(const std::vector<uint8_t> &frame,
                        const uint8_t command, const uint32_t frame_index) {
  for (auto &session : sessions_) {
    if (session.second->command_ == command) {
      // command 1 stream realtime
      if (command == 1u) {
        session.second->doWrite(frame);
      } else {
        if (frame_index == 0u) {
          session.second->command_ = 0u;
          session.second->frame_index_ = 0u;
        } else {
          if (frame_index > session.second->frame_index_) {
            session.second->frame_index_ = frame_index;
            session.second->doWrite(frame);
          }
        }
      }
    }
  }
}

void Session::execute() {
  if (read_buffer_.size()) {
    const auto *data = static_cast<const uint8_t *>(read_buffer_.data().data());
    command_ = *data;
    read_buffer_.consume(read_buffer_.size());
    if (command_ > 1) {
      CameraService::streamRecording(command_, id_);
    }
  }
}
static std::array<uint8_t, 4> serialize_uint32_t(uint32_t size) {
  return std::array<uint8_t, 4>{static_cast<uint8_t>((size >> 24) & 0xFF),
                                static_cast<uint8_t>((size >> 16) & 0xFF),
                                static_cast<uint8_t>((size >> 8) & 0xFF),
                                static_cast<uint8_t>(size & 0xFF)};
}
void Session::doBuff() {
  if (write_buf.size()) {
    auto self(shared_from_this());
    web_socket_.async_write(
        boost::asio::buffer(write_buf, write_buf.size()),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (ec) {
            std::cerr << ec.message() << std::endl;
            web_socket_.async_close(boost::beast::websocket::close_code::normal,
                                    [](boost::beast::error_code ec) {});
            sessions_.extract(id_);
          } else {
            write_buf.erase(write_buf.begin(), write_buf.begin() + length);
            doBuff();
            is_writing_ = false;
          }
        });
    is_writing_ = true;
  }
}
void Session::doWrite(const std::vector<uint8_t> &packet_data) {

  const auto size = serialize_uint32_t(packet_data.size());
  write_buf.insert(write_buf.end(), size.cbegin(), size.cend());
  write_buf.insert(write_buf.end(), packet_data.cbegin(), packet_data.cend());

  if (!is_writing_) {
    doBuff();
  }
}

Server::Server(boost::asio::io_context &io_context, unsigned short port)
    : acceptor_(io_context, boost::asio::ip::tcp::tcp::endpoint(
                                boost::asio::ip::tcp::tcp::v4(), port)) {
  doAccept();
}
void Server::doAccept() {
  acceptor_.async_accept(
      [this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
          std::shared_ptr<Session>(new Session(std::move(socket)))->start();
        }
        doAccept();
      });
}
