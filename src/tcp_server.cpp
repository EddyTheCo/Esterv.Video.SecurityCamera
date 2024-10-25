#include "tcp_server.hpp"
#include "camera_service.hpp"
#include <boost/log/trivial.hpp>


using boost::asio::ip::tcp;

std::unordered_map<size_t, std::shared_ptr<Session>> Session::sessions_;
uint32_t Session::index_{0};

Session::Session(boost::asio::ip::tcp::tcp::socket socket)
    : socket_(std::move(socket)) { }

void Session::doRead() {
    BOOST_LOG_TRIVIAL(debug)
    << __PRETTY_FUNCTION__ ;
    auto self(shared_from_this());
    sessions_.insert({index_, self});
    id_=index_;
    index_++;
    socket_.async_read_some(
        boost::asio::buffer(&command_, 1),
        [this, self](boost::system::error_code ec, std::size_t length) {
            BOOST_LOG_TRIVIAL(debug)
            << "async_read_some";
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

void Session::sendFrame(const std::vector<uint8_t> &frame, const uint8_t command, const uint32_t frame_index)
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

    BOOST_LOG_TRIVIAL(debug)
    << __PRETTY_FUNCTION__ << " " << (int)command_;
    if(command_>1)
    {
        CameraService::streamRecording(command_,id_);
    }
}
static std::array<uint8_t,4> serialize_uint32_t(uint32_t size)
{
    BOOST_LOG_TRIVIAL(debug)
    << __PRETTY_FUNCTION__ << " " << size;
    return std::array<uint8_t,4>{static_cast<uint8_t>((size >> 24) & 0xFF),
                                static_cast<uint8_t>((size >> 16) & 0xFF),
                                static_cast<uint8_t>((size >> 8) & 0xFF),
                                static_cast<uint8_t>(size & 0xFF)};
}
void Session::doWrite(const std::vector<uint8_t> &packet_data) {

    const auto size=serialize_uint32_t(packet_data.size());
    std::vector<uint8_t> buf{size.cbegin(),size.cend()};
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
                                boost::asio::ip::tcp::tcp::v4(), port)) {doAccept();}
void Server::doAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                const auto session =
                    std::shared_ptr<Session>(new Session(std::move(socket)));
                session->doRead();
            }
            doAccept();
        });
}
