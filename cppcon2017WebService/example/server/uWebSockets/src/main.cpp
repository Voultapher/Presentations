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

void broadcastResponse(
  uWS::Hub& h,
  const FlatBufferRef buffer
) {
  h.getDefaultGroup<uWS::SERVER>().broadcast(
    reinterpret_cast<const char*>(buffer.data),
    buffer.size,
    uWS::OpCode::BINARY
  );
}

int main()
{
  std::cout << "Sever starting up...\n";

  PollData<HippieVoteGuard> poll_data{};

  uWS::Hub h;

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

    const auto request = flatbuffers::GetRoot<Strawpoll::Request>(message);

    switch(request->type()) {
      case Strawpoll::RequestType_Poll:
        sendResponse(ws, poll_data.poll_response.ref());
        break;
      case Strawpoll::RequestType_Result:
        poll_data.register_vote(
          request->vote(),
          {},
          [&ws](const FlatBufferRef br) { sendResponse(ws, br); },
          [&h](const FlatBufferRef br) { broadcastResponse(h, br); }
        );
        break;
      default:
        sendResponse(ws, poll_data.error_responses.invalid_type.ref());
    }
  });

  // connect to port and exit if blocked
  if (!h.listen(3003)) return 1;
  h.run();
}
