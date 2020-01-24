set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/misc/cmake;${CMAKE_MODULE_PATH}")
project(uWebSockets)

set(UWS_BACKEND "EPOLL" CACHE STRING "Backend to use. Can be ASIO, EPOLL, LIBUV")

# Dependencies
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

# Define the uWS target
add_library(uWS
  uWebSockets/src/Extensions.cpp
  uWebSockets/src/Group.cpp
  uWebSockets/src/Networking.cpp
  uWebSockets/src/Hub.cpp
  uWebSockets/src/Node.cpp
  uWebSockets/src/WebSocket.cpp
  uWebSockets/src/HTTPSocket.cpp
  uWebSockets/src/Socket.cpp
  uWebSockets/src/Epoll.cpp
)

set(HEADERS
  uWebSockets/src/Asio.h
  uWebSockets/src/Backend.h
  uWebSockets/src/Epoll.h
  uWebSockets/src/Extensions.h
  uWebSockets/src/Group.h
  uWebSockets/src/HTTPSocket.h
  uWebSockets/src/Hub.h
  uWebSockets/src/Libuv.h
  uWebSockets/src/Networking.h
  uWebSockets/src/Node.h
  uWebSockets/src/Socket.h
  uWebSockets/src/uWS.h
  uWebSockets/src/WebSocket.h
  uWebSockets/src/WebSocketProtocol.h
)

target_include_directories(uWS
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(${UWS_BACKEND} MATCHES "ASIO")
  find_package(Boost REQUIRED COMPONENTS system)
  target_compile_definitions(uWS PUBLIC USE_ASIO)
  target_link_libraries(uWS PUBLIC Boost::boost Boost::system)
elseif(${UWS_BACKEND} MATCHES "LIBUV")
  find_package(LibUV REQUIRED)
  target_compile_definitions(uWS PUBLIC USE_LIBUV)
  target_include_directories(uWS PUBLIC "${LIBUV_INCLUDE_DIRS}")
  target_link_libraries(uWS PUBLIC "${LIBUV_LIBRARIES}")
elseif(${UWS_BACKEND} MATCHES "EPOLL")
  target_compile_definitions(uWS PUBLIC USE_EPOLL)
endif()

target_link_libraries(uWS
  PUBLIC
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

target_compile_features(uWS
  PUBLIC
    cxx_std_11
)

set_target_properties(uWS
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    PUBLIC_HEADER "${HEADERS}"
)

install(
  TARGETS uWS
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/uWS
)
