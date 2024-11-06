cmake_minimum_required(VERSION 3.10)

option(BUILD_CLIENT "Build Client part" ON)
option(BUILD_SERVER "Build Server part" ON)

# Project name and version
project(SecurityCamera VERSION 1.0)

include(FetchContent)
# Find OpenCV package
find_package(OpenCV REQUIRED)

if(BUILD_SERVER)
  find_package(Boost REQUIRED COMPONENTS system log)
  add_executable(server src/server.cpp src/tcp_server.cpp src/tcp_server.hpp
                        src/camera_service.hpp src/camera_service.cpp)
  target_link_libraries(server PRIVATE ${OpenCV_LIBS} Boost::system Boost::log)

  set_property(TARGET server PROPERTY CXX_STANDARD 17)
  install(TARGETS server COMPONENT server)
endif(BUILD_SERVER)

if(BUILD_CLIENT)
  include(InstallRequiredSystemLibraries)
  FetchContent_Declare(
      EstervDesigns
      GIT_REPOSITORY https://github.com/EddyTheCo/Esterv.Designs.git
      GIT_TAG develop
      FIND_PACKAGE_ARGS 2 COMPONENTS SimpleStyle FlatControl CONFIG)
    FetchContent_MakeAvailable(EstervDesigns)
  if(EMSCRIPTEN OR ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS QuickControls2)
  endif(EMSCRIPTEN OR ANDROID)
  find_package(Qt6 REQUIRED COMPONENTS Core Network Gui Quick)
  qt_standard_project_setup()
  add_executable(client src/client.cpp src/tcp_client.hpp src/tcp_client.cpp)
  target_include_directories(client PRIVATE src)
  target_link_libraries(client PRIVATE ${OpenCV_LIBS} Qt6::Core Qt6::Network
                                       Qt6::Gui Qt6::Quick Esterv::SimpleStyle)
  set_target_properties(client PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE ON)

  qt6_add_qml_module(
    client
    URI
    SecurityCamera
    VERSION
    1.0
    QML_FILES
    "qml/Client.qml"
    RESOURCE_PREFIX
    "/esterVtech.com/imports"
    IMPORT_PATH
    ${CMAKE_BINARY_DIR})

  install(
    TARGETS client
    COMPONENT Client
    BUNDLE DESTINATION . COMPONENT Client
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Client)
  if(QTDEPLOY)
    qt_generate_deploy_qml_app_script(TARGET client OUTPUT_SCRIPT deploy_script)
    install(SCRIPT ${deploy_script} COMPONENT Client)
  endif(QTDEPLOY)

  if(EMSCRIPTEN OR ANDROID)
    target_link_libraries(
      client
      PRIVATE
        Esterv::FlatControl
        Qt6::QuickControls2
        $<$<STREQUAL:$<TARGET_PROPERTY:Esterv::FlatControl,TYPE>,STATIC_LIBRARY>:Esterv::FlatControlplugin>
    )
    target_compile_definitions(client
                               PRIVATE FORCE_STYLE="Esterv.Controls.Flat")
  endif(EMSCRIPTEN OR ANDROID)
endif(BUILD_CLIENT)
