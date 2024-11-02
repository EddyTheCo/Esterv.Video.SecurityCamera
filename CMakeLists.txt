cmake_minimum_required(VERSION 3.10)

option(BUILD_CLIENT "Build Client part" ON)
option(BUILD_SERVER "Build Server part" ON)

# Project name and version
project(SecurityCamera VERSION 1.0)

# Find OpenCV package
find_package(OpenCV REQUIRED)

find_package(Boost REQUIRED COMPONENTS system log)

if(BUILD_SERVER)
  add_executable(server src/server.cpp src/tcp_server.cpp src/tcp_server.hpp
                        src/camera_service.hpp src/camera_service.cpp)
  target_link_libraries(server PRIVATE ${OpenCV_LIBS} Boost::system Boost::log)
  find_package(Qt6 COMPONENTS Core Network Gui Quick)
  qt_standard_project_setup()
  set_property(TARGET server PROPERTY CXX_STANDARD 17)
  install(TARGETS server COMPONENT server)
endif(BUILD_SERVER)

if(BUILD_CLIENT)
  add_executable(client src/client.cpp src/tcp_client.hpp src/tcp_client.cpp)
  target_link_libraries(client PRIVATE ${OpenCV_LIBS} Qt6::Core Qt6::Network
                                       Qt6::Gui Qt6::Quick)
  set_target_properties(client PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE ON)
  install(TARGETS client COMPONENT client)
endif(BUILD_CLIENT)
