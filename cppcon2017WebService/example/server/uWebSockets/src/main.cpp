#include <uWS/uWS.h>

#include <array>
#include <cstdint>

#include <vector>
#include <string>

#include <strawpoll_generated.h>

#include <iostream> // DEBUG

struct PollData
{
  static constexpr auto title = "When will C++ become obsolete?";
  static constexpr auto options = {
    "Around 2050",
    "Once all the cool kids use ...",
    "Never",
    "AI has no use for high level abstractions",
    "Turnip"
  };
  using vote_t = int64_t;
  using votes_t = std::array<vote_t, options.size()>;
  votes_t votes{};
};

template<typename T, size_t N> void handleInvalidRequest(
  uWS::WebSocket<uWS::SERVER>* ws,
  T (&msg)[N]
) {
  flatbuffers::FlatBufferBuilder builder((sizeof(T) * N) + 64);
  Strawpoll::ResponseBuilder res(builder);
  res.add_type(Strawpoll::ResponseType_Error);
  res.add_error(builder.CreateString(msg));
  builder.Finish(res.Finish());

  ws->send(
    reinterpret_cast<const char*>(builder.GetBufferPointer()),
    builder.GetSize(),
    uWS::OpCode::BINARY
  );
}

void handlePollRequest(
  uWS::WebSocket<uWS::SERVER>* ws,
  flatbuffers::FlatBufferBuilder& poll_builder
) {
  ws->send(
    reinterpret_cast<const char*>(poll_builder.GetBufferPointer()),
    poll_builder.GetSize(),
    uWS::OpCode::BINARY
  );
}

void handleResultRequest(
  uWS::Hub& h,
  uWS::WebSocket<uWS::SERVER>* ws,
  PollData::votes_t& votes,
  PollData::vote_t vote
) {
  if (vote < 0 || vote > static_cast<PollData::vote_t>(PollData::options.size()))
    handleInvalidRequest(ws, "Invalid vote");

  ++votes[vote];

  flatbuffers::FlatBufferBuilder builder(sizeof(votes) + 64);
  const auto result = Strawpoll::CreateResult(
    builder,
    builder.CreateVector(votes.data(), votes.size())
  );

  Strawpoll::ResponseBuilder res(builder);
  res.add_type(Strawpoll::ResponseType_Result);
  res.add_result(result);
  builder.Finish(res.Finish());

  //std::cout << "Builder Size: " << builder.GetSize() << '\n';

  h.getDefaultGroup<uWS::SERVER>().broadcast(
    reinterpret_cast<const char*>(builder.GetBufferPointer()),
    builder.GetSize(),
    uWS::OpCode::BINARY
  );
}

int main()
{
  std::cout << "Sever starting up...\n";

  auto poll_data = PollData{};

  flatbuffers::FlatBufferBuilder poll_builder;
  {
    const auto poll = Strawpoll::CreatePoll(
      poll_builder,
      poll_builder.CreateString(poll_data.title),
      poll_builder.CreateVectorOfStrings([&poll_data]{
        std::vector<std::string> options;
        for (const auto& option : poll_data.options)
          options.emplace_back(option);
        return options;
      }())
    );

    Strawpoll::ResponseBuilder res(poll_builder);
    res.add_type(Strawpoll::ResponseType_Poll);
    res.add_poll(poll);

    poll_builder.Finish(res.Finish());
  }

  uWS::Hub h;

  /*h.onConnection([&h, &poll_data](
    uWS::WebSocket<uWS::SERVER> *ws,
    uWS::HttpRequest req
  ) {
    std::cout << "Someone connected\n";
  });*/

  h.onMessage([&h, &poll_data, &poll_builder](
    uWS::WebSocket<uWS::SERVER>* ws,
    char* message,
    size_t length,
    uWS::OpCode
  ) {
    const auto ok = flatbuffers::Verifier(
      reinterpret_cast<const uint8_t*>(message), length
    ).VerifyBuffer<Strawpoll::Request>(nullptr);

    if (!ok) {
      handleInvalidRequest(ws, "Invalid request message");
      return;
    }

    // TODO limit voting from single ip
    std::cout << "Message from: " << ws->getAddress().address << '\n';

    const auto request = flatbuffers::GetRoot<Strawpoll::Request>(message);

    switch(request->type()) {
      case Strawpoll::RequestType_Poll:
        handlePollRequest(ws, poll_builder);
        break;
      case Strawpoll::RequestType_Result:
        handleResultRequest(h, ws, poll_data.votes, request->vote());
        break;
      default: handleInvalidRequest(ws, "Invalid request type");
    }
  });

  // connect to port and exit if blocked
  if (!h.listen(3003)) return 1;
  h.run();
}
