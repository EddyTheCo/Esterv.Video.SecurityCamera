#include <boost/asio/io_context.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <thread>


enum class ReadingState { Size, Data };
// The data size can be changed depending on the available memory
enum MAX_SIZES : size_t { Data = 10000 };
static std::array<uint8_t, MAX_SIZES::Data> data_received;

std::string server_ip;
uint32_t server_port;

static uint32_t deserialize_uint32_t(const std::array<uint8_t, 4>& buf) {
    BOOST_LOG_TRIVIAL(debug)
    << __PRETTY_FUNCTION__ << " buf[0]:" << (int)buf[0]<< " buf[1]:"<<(int)buf[1]<< " buf[2]:"<<(int)buf[2]<< " buf[3]:"<<(int)buf[3];
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) << 8)  |
           static_cast<uint32_t>(buf[3]);
}

void receiveFrame(const std::size_t length) {
    BOOST_LOG_TRIVIAL(debug) << __PRETTY_FUNCTION__<< length;
    static uint32_t frameSize{0};
    static std::vector<uint8_t> frame;
    static std::array<uint8_t,4> size;
    static uint8_t size_pos{0};
    static ReadingState state{ReadingState::Size};

    std::size_t pos{0};

    while (pos < length) {

        switch (state) {
        case ReadingState::Size:
            size.at(size_pos)=data_received.at(pos);

            BOOST_LOG_TRIVIAL(debug)<<"size.at(size_pos):"<<(int)size.at(size_pos)<<" data_received.at(pos):"<<(int)data_received.at(pos);
            if(++size_pos == 4)
            {
                frameSize = deserialize_uint32_t(size);
                BOOST_LOG_TRIVIAL(debug) << "frameSize:"<< frameSize;
                state=ReadingState::Data;
                size_pos=0;
            }

            ++pos;
            break;
        case ReadingState::Data:
            const auto take_size =
                std::min(frameSize - frame.size(), length - pos);
            std::move(data_received.begin() + pos, data_received.begin() + pos + take_size,
                      std::back_inserter(frame));
            pos += take_size;
            if (frame.size() == frameSize) {
                BOOST_LOG_TRIVIAL(debug) << "frame.size():"<< frame.size();
                const auto grayImage= cv::imdecode(frame, cv::IMREAD_GRAYSCALE);
                std::cout << "grayImage:" <<grayImage.size()<< std::endl;
                cv::imshow("Grayscale Image", grayImage);
                cv::waitKey(1);

                state = ReadingState::Size;
                frame.clear();
            }
            break;
        }
    }
}

void tryToConnect(boost::asio::ip::tcp::socket& socket);
void doClientRead(boost::asio::ip::tcp::socket& socket) {
    socket.async_read_some(
        boost::asio::buffer(data_received, MAX_SIZES::Data),
        [&](boost::system::error_code ec, std::size_t length) {
            if (!ec) {

                receiveFrame(length);
                doClientRead(socket);

            } else {
                BOOST_LOG_TRIVIAL(error)
                << __PRETTY_FUNCTION__ << " " << ec.message();
                tryToConnect(socket);
            }
        });
}
void tryToConnect(boost::asio::ip::tcp::socket& socket) {
    socket.close();
    const boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::address::from_string(server_ip),
        server_port);
    try {
        socket.connect(endpoint);
        doClientRead(socket);
    } catch (...) {
        BOOST_LOG_TRIVIAL(info) << "There is no LAN Server running at "
                                << server_ip << ":" << server_port;
        BOOST_LOG_TRIVIAL(info) << "Retrying in 5 seconds";
        std::this_thread::sleep_for(std::chrono::seconds(5));
        tryToConnect(socket);
    }
}


int main(int argc, char *argv[])
{
    boost::asio::io_context io_context;

    server_ip = "127.0.0.1";
    server_port = 9999;

    boost::asio::ip::tcp::socket socket(io_context);
    tryToConnect(socket);

    boost::system::error_code ec;

    uint8_t command{1u};
    try {
        socket.write_some(boost::asio::buffer(&command,1),ec);
    } catch (...) {
        BOOST_LOG_TRIVIAL(info) << ec.message();
        tryToConnect(socket);
    }

    io_context.run();
}
