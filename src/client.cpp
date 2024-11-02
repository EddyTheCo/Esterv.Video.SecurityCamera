#include <boost/asio/io_context.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>



int main(int argc, char *argv[])
{
    boost::asio::io_context io_context;

    boost::asio::ip::tcp::socket socket(io_context);
    boost::asio::ip::tcp::resolver resolver{io_context};


    tryToConnect(socket,resolver);

    boost::system::error_code ec;

    uint8_t command{1u};
    try {
        socket.write_some(boost::asio::buffer(&command,1),ec);
    } catch (...) {
        BOOST_LOG_TRIVIAL(info) << ec.message();
        tryToConnect(socket,resolver);
    }

    io_context.run();
}
