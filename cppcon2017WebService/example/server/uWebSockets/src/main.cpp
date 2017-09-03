#include <uWS/uWS.h>

#include <strawpoll.hpp>

#include <iostream> // DEBUG

void sendResponse(
  uWS::WebSocket<uWS::SERVER>* ws,
  const FlatBufferRef buffer
) {
  ws->send(
    reinterpret_cast<const char*>(buffer.data),
    buffer.size,
    uWS::OpCode::BINARY
  );
}

void handleResultRequest(
  uWS::Hub& h,
  uWS::WebSocket<uWS::SERVER>* ws,
  PollData& poll_data,
  const PollData::vote_t vote
) {
  if (vote < 0 || vote > static_cast<PollData::vote_t>(PollData::options.size()))
    sendResponse(ws, poll_data.error_responses.invalid_type.ref());

  ++poll_data.votes[vote];

  const FlatBufferWrapper result{
    [&poll_data](flatbuffers::FlatBufferBuilder& builder) -> void
    {
      const auto result = Strawpoll::CreateResult(
        builder,
        builder.CreateVector(poll_data.votes.data(), poll_data.votes.size())
      );

      Strawpoll::ResponseBuilder res(builder);
      res.add_type(Strawpoll::ResponseType_Result);
      res.add_result(result);
      builder.Finish(res.Finish());
    }
  };

  const auto result_ref = result.ref();

  h.getDefaultGroup<uWS::SERVER>().broadcast(
    reinterpret_cast<const char*>(result_ref.data),
    result_ref.size,
    uWS::OpCode::BINARY
  );
}

int main()
{
  std::cout << "Sever starting up...\n";

  auto poll_data = PollData{};

  uWS::Hub h;

  /*h.onConnection([&h, &poll_data](
    uWS::WebSocket<uWS::SERVER> *ws,
    uWS::HttpRequest req
  ) {
    std::cout << "Someone connected\n";
  });*/

  h.onMessage([&h, &poll_data](
    uWS::WebSocket<uWS::SERVER>* ws,
    char* message,
    size_t length,
    uWS::OpCode
  ) {
    const auto ok = flatbuffers::Verifier(
      reinterpret_cast<const uint8_t*>(message), length
    ).VerifyBuffer<Strawpoll::Request>(nullptr);

    if (!ok) {
      sendResponse(ws, poll_data.error_responses.invalid_message.ref());
      return;
    }

    // TODO limit voting from single ip
    std::cout << "Message from: " << ws->getAddress().address << '\n';

    const auto request = flatbuffers::GetRoot<Strawpoll::Request>(message);

    switch(request->type()) {
      case Strawpoll::RequestType_Poll:
        sendResponse(ws, poll_data.poll_response.ref());
        break;
      case Strawpoll::RequestType_Result:
        handleResultRequest(h, ws, poll_data, request->vote());
        break;
      default:
        sendResponse(ws, poll_data.error_responses.invalid_type.ref());
    }
  });

  // connect to port and exit if blocked
  if (!h.listen(3003)) return 1;
  h.run();
}
