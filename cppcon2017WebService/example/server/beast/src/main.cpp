//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, synchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/functional/hash.hpp>
//#include <boost/asio/buffers_iterator.hpp>
//#include <cstdlib>
//#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <array>
#include <strawpoll.hpp>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

//------------------------------------------------------------------------------

namespace detail
{
  template<typename T> struct hash
  {
    size_t operator() (const T& v) const noexcept
    {
      if (v.is_v4()) return v.to_v4().to_ulong();
      if (v.is_v6())
      {
          auto const& range = v.to_v6().to_bytes();
          return boost::hash_range(range.begin(), range.end());
      }
      if (v.is_unspecified()) return 0x4751301174351161ul;
      return boost::hash_value(v.to_string());
    }
  };
}

void sendResponse(
  websocket::stream<tcp::socket>& ws,
  const FlatBufferRef buffer
) {
  ws.binary(true);
  ws.write(std::array<boost::asio::const_buffer, 1>{{
    { buffer.data, buffer.size }
  }});
}

void broadcastResponse(
  websocket::stream<tcp::socket>& ws,
  const FlatBufferRef buffer
) {
  ws.binary(true);
  ws.write(std::array<boost::asio::const_buffer, 1>{{
    { buffer.data, buffer.size }
  }});
}

// Echoes back all received WebSocket messages
template<typename T> void do_session(
  tcp::socket&& socket,
  PollData<T>& poll_data
) {
  try
  {
    // Construct the stream by moving in the socket
    //socket.binary
    const auto address = socket.remote_endpoint().address();
    websocket::stream<tcp::socket> ws{std::move(socket)};

    // Accept the websocket handshake
    ws.accept();

    for(;;)
    {
      // This buffer will hold the incoming message
      boost::beast::multi_buffer buffers;
      ws.read(buffers);

      for (const auto buffer : buffers.data())
      {
        const auto ok = flatbuffers::Verifier(
          boost::asio::buffer_cast<const uint8_t*>(buffer),
          boost::asio::buffer_size(buffer)
        ).VerifyBuffer<Strawpoll::Request>(nullptr);

        if (!ok) {
          sendResponse(ws, poll_data.error_responses.invalid_message.ref());
          return;
        }

        const auto request = flatbuffers::GetRoot<Strawpoll::Request>(
          boost::asio::buffer_cast<const uint8_t*>(buffer)
        );

        switch(request->type())
        {
          case Strawpoll::RequestType_Poll:
            if (poll_data.vote_guard.has_voted(address))
              sendResponse(ws, make_result(poll_data.votes).ref());

            sendResponse(ws, poll_data.poll_response.ref());
            break;
          case Strawpoll::RequestType_Result:
            poll_data.register_vote(
              request->vote(),
              address,
              [&ws](const FlatBufferRef br) { sendResponse(ws, br); },
              [&ws](const FlatBufferRef br) { broadcastResponse(ws, br); }
            );
            break;
          default:
            sendResponse(ws, poll_data.error_responses.invalid_type.ref());
        }
      }
    }
  }
  catch(const boost::system::system_error& se)
  {
      // This indicates that the session was closed
      if(se.code() != websocket::error::closed)
        std::cerr << "Error: " << se.code().message() << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

//------------------------------------------------------------------------------

int main()
{
  try
  {
    PollData<VoteGuard<boost::asio::ip::address, detail::hash>> poll_data{};

    const auto address = boost::asio::ip::address::from_string("127.0.0.1");
    const auto port = static_cast<unsigned short>(std::atoi("3003"));

    // The io_service is required for all I/O
    boost::asio::io_service ios{1};

    // The acceptor receives incoming connections
    tcp::acceptor acceptor{ios, {address, port}};
    for(;;)
    {
      // This will receive the new connection
      tcp::socket socket{ios};

      // Block until we get a connection
      acceptor.accept(socket);
      // Launch the session, transferring ownership of the socket
      std::thread{
        [sock = std::move(socket), &poll_data]() mutable
        {
          do_session(std::move(sock), poll_data);
        }
      }.detach();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
