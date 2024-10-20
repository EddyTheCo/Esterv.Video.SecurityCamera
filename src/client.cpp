#include <boost/asio/io_context.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <thread>


enum class ReadingState { Size, Data };
enum MAX_SIZES : size_t { Data = 65000 };
static std::array<uint8_t, MAX_SIZES::Data> data_received;

std::string server_ip;
uint32_t server_port;

void receiveFrame(const std::size_t length) {
    BOOST_LOG_TRIVIAL(debug) << __PRETTY_FUNCTION__<< length;
    static uint32_t frameSize{0};
    static std::vector<uint8_t> frame;
    static std::vector<uint8_t> size;
    static ReadingState state{ReadingState::Size};

    std::size_t pos{0};
    while (pos < length) {
        BOOST_LOG_TRIVIAL(debug)<<"data_received:"<<(int)data_received.at(pos)<< " pos:"<<pos;
        switch (state) {
        case ReadingState::Size:

            size.push_back(data_received.at(pos));
            if(size.size() == 2)
            {
                frameSize = (static_cast<uint16_t>(size.at(0) & 0xFF) << 8);
                frameSize = frameSize | (size.at(1) & 0xFF);
                BOOST_LOG_TRIVIAL(debug) << "frameSize:"<< frameSize;
                size.clear();
                state=ReadingState::Data;
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
                cv::waitKey(0);
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
