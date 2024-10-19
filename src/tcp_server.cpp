#include "tcp_server.hpp"
#include "camera_service.hpp"
#include <boost/log/trivial.hpp>


using boost::asio::ip::tcp;

std::unordered_map<size_t, std::shared_ptr<Session>> Session::sessions_;
uint32_t Session::index_{0};

Session::Session(boost::asio::ip::tcp::tcp::socket socket)
    : socket_(std::move(socket)) { doRead();}

void Session::doRead() {
    auto self(shared_from_this());
    sessions_.insert({index_, self});
    id_=index_;
    index_++;
    socket_.async_read_some(
        boost::asio::buffer(&command_, 4),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                execute();
                doRead();
            } else {
                BOOST_LOG_TRIVIAL(error)
                << __PRETTY_FUNCTION__ << " " << ec.message();
                socket_.close();
                sessions_.extract(id_);
            }
        });
}

void Session::sendFrame(const std::vector<uint8_t> &frame, const uint32_t command, const uint32_t frame_index)
{
    for(auto & session:sessions_)
    {
        if(session.second->command_==command)
        {
            //command 1 stream realtime
            if(command==1u)
            {
                session.second->doWrite(frame);
            }
            else
            {
                if(frame_index==0u)
                {
                    session.second->command_=0u;
                    session.second->frame_index_=0u;
                }
                else
                {
                    if(frame_index>session.second->frame_index_)
                    {
                        session.second->frame_index_=frame_index;
                        session.second->doWrite(frame);
                    }
                }
            }
        }
    }
}

void Session::execute()
{
    if(command_>1)
    {
        CameraService::streamRecording(command_,id_);
    }
}

void Session::doWrite(const std::vector<uint8_t> &packet_data) {
    std::vector<uint8_t> buf{};
    buf.insert(buf.end(), packet_data.begin(), packet_data.end());
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_, boost::asio::buffer(buf, buf.size()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (ec) {
                BOOST_LOG_TRIVIAL(error)
                << __PRETTY_FUNCTION__ << " " << ec.message();
                socket_.close();
                sessions_.extract(id_);
            }
        });
}

Server::Server(boost::asio::io_context &io_context, unsigned short port)
    : acceptor_(io_context, boost::asio::ip::tcp::tcp::endpoint(
                                boost::asio::ip::tcp::tcp::v4(), port)) {}
void Server::doAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                const auto session = Session(std::move(socket));
            }
            doAccept();
        });
}
