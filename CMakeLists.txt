cmake_minimum_required(VERSION 3.24)

include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)

option(BUILD_CLIENT "Build Client part" ON)
option(BUILD_SERVER "Build Server part" ON)
option(BUILD_DOCS "Build Documentation part" OFF)
option(QTDEPLOY "Deploy Qt libraries" OFF)
option(MOTION_DETECTION "Add motion detection and save videos on the server"
       OFF)

include(FetchContent)
FetchContent_Declare(
  ccommon
  GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
  GIT_TAG ndk26)
FetchContent_MakeAvailable(ccommon)
version_from_git(LOG OFF TIMESTAMP "%Y%m%d%H%M%S")
project(
  EstervSecurityCamera
  VERSION ${VERSION}
  DESCRIPTION "TCP Server Client of a camera stream"
  LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(GNUInstallDirs)

if(BUILD_SERVER
   AND NOT EMSCRIPTEN
   AND NOT ANDROID)
  find_package(
    OpenCV
    COMPONENTS core imgproc videoio video
    )

  if(NOT OpenCV_FOUND)
	set(builturl "https://github.com/EddyTheCo/install-OpenCV-action/releases/latest/download/OpenCV-ubuntu-latest-wasm_false-android_none.tar")
	if(WIN32)
		set(builturl "https://github.com/EddyTheCo/install-OpenCV-action/releases/latest/download/OpenCV-windows-latest-wasm_false-android_none.tar")
	endif(WIN32)
	if (APPLE)
		set(builturl "https://github.com/EddyTheCo/install-OpenCV-action/releases/latest/download/OpenCV-macos-latest-wasm_false-android_none.tar")
	endif(APPLE)
	FetchContent_Declare(
		OpenCV
		DOWNLOAD_EXTRACT_TIMESTAMP true
		URL  ${builturl}
		)
	FetchContent_GetProperties(OpenCV)
	if(NOT OpenCV_POPULATED)
		message(STATUS "OpenCV prebuilt will be downloaded from ${builturl} if not found on the system")
		FetchContent_Populate(OpenCV)
		message(STATUS "OpenCV is installed in ${opencv_SOURCE_DIR}")
		set(OpenCV_DIR ${opencv_SOURCE_DIR}/lib/cmake/opencv4)
		if(WIN32)
			set(OpenCV_DIR ${opencv_SOURCE_DIR})
		endif(WIN32)
		find_package(OpenCV COMPONENTS core imgproc videoio video)
	endif()
	if(OpenCV_FOUND AND NOT ANDROID AND NOT EMSCRIPTEN)
		install(IMPORTED_RUNTIME_ARTIFACTS ${OpenCV_LIBS}
			DESTINATION ${CMAKE_INSTALL_LIBDIR}
			COMPONENT server
			)
	endif(OpenCV_FOUND AND NOT ANDROID AND NOT EMSCRIPTEN)
endif()

FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.86.0
  FIND_PACKAGE_ARGS
  1.83
  COMPONENTS
  system log
  CONFIG)
FetchContent_MakeAvailable(Boost)

  add_executable(server src/server.cpp src/tcp_server.cpp src/tcp_server.hpp
                        src/camera_service.hpp src/camera_service.cpp)
  target_link_libraries(server PRIVATE ${OpenCV_LIBS} Boost::system Boost::log)

  if(MOTION_DETECTION)
    target_compile_definitions(server PRIVATE MOTION_DETECTION)
  endif(MOTION_DETECTION)
  set_property(TARGET server PROPERTY CXX_STANDARD 17)
  install(TARGETS server COMPONENT server)
endif(
  BUILD_SERVER
  AND NOT EMSCRIPTEN
  AND NOT ANDROID)

if(BUILD_CLIENT)
  include(InstallRequiredSystemLibraries)

  FetchContent_Declare(
    EstervDesigns
    GIT_REPOSITORY https://github.com/EddyTheCo/Esterv.Designs.git
    GIT_TAG develop
    FIND_PACKAGE_ARGS
    2
    COMPONENTS
    SimpleStyle
    CustomControls
    FlatControl
    EstervAbout${CMAKE_PROJECT_NAME}
    CONFIG)
  FetchContent_MakeAvailable(EstervDesigns)
  if(EMSCRIPTEN OR ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS QuickControls2)
  endif(EMSCRIPTEN OR ANDROID)
  find_package(
    Qt6 REQUIRED
    COMPONENTS Core Gui Quick
    OPTIONAL_COMPONENTS WebSockets)
  qt_standard_project_setup()
  if(NOT TARGET Qt6::WebSockets)
    FetchContent_Declare(
      qtwebsockets
      GIT_REPOSITORY https://github.com/qt/qtwebsockets.git
      GIT_TAG 6.6.0)
    FetchContent_MakeAvailable(qtwebsockets)
  endif(NOT TARGET Qt6::WebSockets)

  qt_add_executable(client src/client.cpp src/tcp_client.hpp src/tcp_client.cpp)
  target_include_directories(client PRIVATE src)
  target_link_libraries(client PRIVATE ${OpenCV_LIBS} Qt6::Core Qt6::WebSockets
                                       Qt6::Gui Qt6::Quick Esterv::SimpleStyle Esterv::About${CMAKE_PROJECT_NAME}
				   $<$<STREQUAL:$<TARGET_PROPERTY:Esterv::About${CMAKE_PROJECT_NAME},TYPE>,STATIC_LIBRARY>:Esterv::About${CMAKE_PROJECT_NAME}plugin>)
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
  if(ANDROID)
	set_property(TARGET client PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
	set_property(TARGET client APPEND PROPERTY QT_ANDROID_MIN_SDK_VERSION 30)
	set_property(TARGET client APPEND PROPERTY QT_ANDROID_TARGET_SDK_VERSION 34)
	set_property(TARGET client APPEND PROPERTY QT_ANDROID_SDK_BUILD_TOOLS_REVISION 34.0.0)

	FetchContent_Declare(
		android_openssl
		DOWNLOAD_EXTRACT_TIMESTAMP true
		URL      https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
		)
	FetchContent_GetProperties(android_openssl)
	if(NOT android_openssl_POPULATED)
		FetchContent_Populate(android_openssl)
		include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
		add_android_openssl_libraries(client)
	endif(NOT android_openssl_POPULATED)
endif(ANDROID)
  if(EMSCRIPTEN)
    add_custom_command(
      TARGET client
      POST_BUILD
      COMMAND
        ${CMAKE_COMMAND} ARGS -E copy "${CMAKE_CURRENT_BINARY_DIR}/client.js"
        "${CMAKE_CURRENT_BINARY_DIR}/client.wasm"
        "${CMAKE_CURRENT_BINARY_DIR}/qtloader.js"
        "${CMAKE_CURRENT_SOURCE_DIR}/wasm")
  endif(EMSCRIPTEN)
endif(BUILD_CLIENT)

build_cpack()
if(BUILD_DOCS)
  build_docs()
endif(BUILD_DOCS)
