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
#include <boost/asio/strand.hpp>
#include <boost/functional/hash.hpp>
//#include <boost/asio/buffers_iterator.hpp>
//#include <cstdlib>
//#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <array>
#include <unordered_map>
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

  template<typename T, typename M> class member_func_ptr_closure
  {
  public:
    using obj_ptr_t = T*;
    using member_func_ptr_t = M;

    constexpr member_func_ptr_closure(
      obj_ptr_t obj_ptr,
      member_func_ptr_t member_func_ptr
    ) noexcept
      : obj_ptr_{obj_ptr}, member_func_ptr_{member_func_ptr}
    {}

    template<typename... Args> auto operator() (Args&&... args)
    {
      return ((obj_ptr_)->*(member_func_ptr_))(std::forward<Args>(args)...);
    }

  private:
    obj_ptr_t obj_ptr_;
    member_func_ptr_t member_func_ptr_;
  };

  namespace conv
  {
    boost::asio::const_buffer m_b(FlatBufferRef buffer)
    {
      return { buffer.data, buffer.size };
    }
  }
} // namespace detail

//using poll_data_t = PollData<VoteGuard<boost::asio::ip::address, detail::hash>>;
using poll_data_t = PollData<HippieVoteGuard<boost::asio::ip::address>>;

// Report a failure
void fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
template<typename FC, typename FB> class session
{
public:
  // Take ownership of the socket
  explicit session(
    tcp::socket&& socket,
    boost::asio::ip::address address,
    FC on_close,
    FB broadcast,
    size_t session_id,
    poll_data_t& poll_data
  )
      : ws_{std::move(socket)}
      , address_{address}
      , strand_{ws_.get_io_service()}
      , read_in_process_{false}
      , write_in_process_{false}
      , on_close_{on_close}
      , broadcast_{broadcast}
      , session_id_{session_id}
      , poll_data_{poll_data}
  {
    ws_.binary(true); // we'll only write binary

    // Accept the websocket handshake
    ws_.async_accept(
        strand_.wrap([this](boost::system::error_code ec) { on_accept(ec); })
    );
  }

  session(const session&) = delete;
  session(session&&) = default;

  session& operator= (const session&) = delete;
  session& operator= (session&&) = default;

  ~session() = default;

  void add_message(FlatBufferRef br)
  {
    if (
      std::is_same_v<
        poll_data_t::vote_guard_t,
        HippieVoteGuard<boost::asio::ip::address>
      > || poll_data_.vote_guard.has_voted(address_)
    )
      message_queue_.push_back(detail::conv::m_b(br));
  }

  void flush_message_queue()
  {
    if (write_in_process_) return;

    if (message_queue_.empty()) return;

    ws_.async_write(
      std::array<boost::asio::const_buffer, 1>{{
        std::move(message_queue_.back())
      }},
      strand_.wrap(
        [this](boost::system::error_code ec, size_t bytes_transferred)
        {
          write_in_process_ = false;
          on_write(ec, bytes_transferred);
        }
      )
    );

    write_in_process_ = true;
    message_queue_.pop_back();
  }

private:
  websocket::stream<tcp::socket> ws_;
  boost::asio::ip::address address_;
  boost::asio::io_service::strand strand_;
  boost::beast::multi_buffer buffer_;
  std::vector<boost::asio::const_buffer> message_queue_;
  bool read_in_process_;
  bool write_in_process_;
  FC on_close_;
  FB broadcast_;
  size_t session_id_;
  poll_data_t& poll_data_;

  void on_accept(boost::system::error_code ec)
  {
    if (ec) return fail(ec, "accept");

    // Read a message
    do_read();
  }

  void do_read()
  {
    if (read_in_process_) return;
    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read a message into our buffer
    ws_.async_read(
      buffer_,
      strand_.wrap(
        [this](boost::system::error_code ec, size_t bytes_transferred)
        {
          read_in_process_ = false;
          on_read(ec, bytes_transferred);
        }
      )
    );
    read_in_process_ = true;
  }

  void on_read(
    boost::system::error_code ec,
    size_t bytes_transferred
  ) {
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
    {
      on_close_(session_id_);
      return;
    }

    if (ec) fail(ec, "read");

    build_responses();
    flush_message_queue();
  }

  void on_write(
    boost::system::error_code ec,
    size_t// bytes_transferred
  ) {
    //boost::ignore_unused(bytes_transferred);

    if (ec) return fail(ec, "write");

    if (!message_queue_.empty())
    {
      flush_message_queue();
      return;
    }

    // Do another read
    do_read();
  }

  void build_responses()
  {
    const auto add_response = [&queue = message_queue_](FlatBufferRef br)
    {
      queue.push_back(detail::conv::m_b(br));
    };

    for (const auto buffer : buffer_.data())
    {
      const auto ok = flatbuffers::Verifier(
        boost::asio::buffer_cast<const uint8_t*>(buffer),
        boost::asio::buffer_size(buffer)
      ).VerifyBuffer<Strawpoll::Request>(nullptr);

      if (!ok) {
        add_response(poll_data_.error_responses.invalid_message.ref());
        continue;
      }

      const auto request = flatbuffers::GetRoot<Strawpoll::Request>(
        boost::asio::buffer_cast<const uint8_t*>(buffer)
      );

      switch(request->type())
      {
        case Strawpoll::RequestType_Poll:
          if (poll_data_.vote_guard.has_voted(address_))
            add_response(poll_data_.result.ref());

          add_response(poll_data_.poll_response.ref());
          break;
        case Strawpoll::RequestType_Result:
          poll_data_.register_vote(
            request->vote(),
            address_,
            add_response,
            [&broadcast = broadcast_](FlatBufferRef br) { broadcast(br); }
          );
          break;
        default:
          add_response(poll_data_.error_responses.invalid_type.ref());
      }
    }
  }
};

// Accepts incoming connections and launches the sessions
class listener
{
public:
  void on_session_close(size_t session_id)
  {
    sessions_.erase(session_id);
  }

  void broadcast(FlatBufferRef br)
  {
    for (auto& [key, session] : sessions_)
    {
      session.add_message(br);
      session.flush_message_queue();
    }
  }

  using on_session_close_t = detail::member_func_ptr_closure<
    listener,
    decltype(&listener::on_session_close)
  >;
  using broadcast_t = detail::member_func_ptr_closure<
    listener,
    decltype(&listener::broadcast)
  >;

  using session_t = session<on_session_close_t, broadcast_t>;
  using sessions_t = std::unordered_map<size_t, session_t>;

  explicit listener(
      boost::asio::io_service& ios,
      tcp::endpoint endpoint)
      : strand_{ios}
      , acceptor_{ios}
      , socket_{ios}
      , session_id_counter_{}
      , on_session_close_{this, &listener::on_session_close}
      , broadcast_{this, &listener::broadcast}
      , poll_data_{}
  {
    boost::system::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
      fail(ec, "open");
      return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec)
    {
      fail(ec, "bind");
      return;
    }

    // Start listening for connections
    acceptor_.listen(boost::asio::socket_base::max_connections, ec);
    if (ec)
    {
      fail(ec, "listen");
      return;
    }

    if (! acceptor_.is_open()) return;
    do_accept();
  }

  listener(const listener&) = delete;
  listener(listener&&) = default;

  listener& operator= (const listener&) = delete;
  listener& operator= (listener&&) = default;

  ~listener() = default;

  void do_accept()
  {
    acceptor_.async_accept(
      socket_,
      strand_.wrap([this](boost::system::error_code ec) { on_accept(ec); })
    );
  }

  void on_accept(boost::system::error_code ec)
  {
    if (ec)
    {
      fail(ec, "accept");
    }
    else
    {
      // Create the session and run it
      const auto address = socket_.remote_endpoint().address();
      sessions_.try_emplace(
        session_id_counter_,
        std::move(socket_),
        std::move(address),
        on_session_close_,
        broadcast_,
        session_id_counter_,
        poll_data_
      );
      ++session_id_counter_;
    }

    // Accept another connection
    do_accept();
  }

private:
  boost::asio::io_service::strand strand_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  size_t session_id_counter_;
  on_session_close_t on_session_close_;
  broadcast_t broadcast_;
  sessions_t sessions_;
  poll_data_t poll_data_;
};

//------------------------------------------------------------------------------

int main()
{
  const auto address = boost::asio::ip::address::from_string("127.0.0.1");
  const auto port = static_cast<unsigned short>(3003);

  boost::asio::io_service ios{1};
  listener lis{ios, tcp::endpoint{address, port}};
  ios.run(); // blocking
}
