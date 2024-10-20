#include"camera_service.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/log/trivial.hpp>
#include"tcp_server.hpp"
int main()
{
    boost::asio::io_context io_context;

    auto lan_server =
        std::make_unique<Server>(io_context, 9999);


    const auto cservice=CameraService();
    io_context.run();
    return 0;
}
