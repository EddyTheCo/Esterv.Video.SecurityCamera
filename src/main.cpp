#include"camera_service.hpp"
#include<thread>
int main()
{
    const auto cservice=CameraService();
    std::this_thread::sleep_for(std::chrono::seconds(30));
    return 0;
}
