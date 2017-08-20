// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_STRAWPOLL_STRAWPOLL_H_
#define FLATBUFFERS_GENERATED_STRAWPOLL_STRAWPOLL_H_

#include "flatbuffers/flatbuffers.h"

namespace Strawpoll {

struct Poll;

struct Result;

struct Request;

struct Response;

enum RequestType {
  RequestType_Poll = 0,
  RequestType_Result = 1,
  RequestType_MIN = RequestType_Poll,
  RequestType_MAX = RequestType_Result
};

inline RequestType (&EnumValuesRequestType())[2] {
  static RequestType values[] = {
    RequestType_Poll,
    RequestType_Result
  };
  return values;
}

inline const char **EnumNamesRequestType() {
  static const char *names[] = {
    "Poll",
    "Result",
    nullptr
  };
  return names;
}

inline const char *EnumNameRequestType(RequestType e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesRequestType()[index];
}

enum ResponseType {
  ResponseType_Poll = 0,
  ResponseType_Votes = 1,
  ResponseType_Error = 2,
  ResponseType_MIN = ResponseType_Poll,
  ResponseType_MAX = ResponseType_Error
};

inline ResponseType (&EnumValuesResponseType())[3] {
  static ResponseType values[] = {
    ResponseType_Poll,
    ResponseType_Votes,
    ResponseType_Error
  };
  return values;
}

inline const char **EnumNamesResponseType() {
  static const char *names[] = {
    "Poll",
    "Votes",
    "Error",
    nullptr
  };
  return names;
}

inline const char *EnumNameResponseType(ResponseType e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesResponseType()[index];
}

struct Poll FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_TITLE = 4,
    VT_OPTIONS = 6
  };
  const flatbuffers::String *title() const {
    return GetPointer<const flatbuffers::String *>(VT_TITLE);
  }
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *options() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_OPTIONS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_TITLE) &&
           verifier.Verify(title()) &&
           VerifyOffset(verifier, VT_OPTIONS) &&
           verifier.Verify(options()) &&
           verifier.VerifyVectorOfStrings(options()) &&
           verifier.EndTable();
  }
};

struct PollBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_title(flatbuffers::Offset<flatbuffers::String> title) {
    fbb_.AddOffset(Poll::VT_TITLE, title);
  }
  void add_options(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> options) {
    fbb_.AddOffset(Poll::VT_OPTIONS, options);
  }
  PollBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  PollBuilder &operator=(const PollBuilder &);
  flatbuffers::Offset<Poll> Finish() {
    const auto end = fbb_.EndTable(start_, 2);
    auto o = flatbuffers::Offset<Poll>(end);
    return o;
  }
};

inline flatbuffers::Offset<Poll> CreatePoll(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> title = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> options = 0) {
  PollBuilder builder_(_fbb);
  builder_.add_options(options);
  builder_.add_title(title);
  return builder_.Finish();
}

inline flatbuffers::Offset<Poll> CreatePollDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *title = nullptr,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *options = nullptr) {
  return Strawpoll::CreatePoll(
      _fbb,
      title ? _fbb.CreateString(title) : 0,
      options ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*options) : 0);
}

struct Result FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_VOTES = 4
  };
  const flatbuffers::Vector<int64_t> *votes() const {
    return GetPointer<const flatbuffers::Vector<int64_t> *>(VT_VOTES);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_VOTES) &&
           verifier.Verify(votes()) &&
           verifier.EndTable();
  }
};

struct ResultBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_votes(flatbuffers::Offset<flatbuffers::Vector<int64_t>> votes) {
    fbb_.AddOffset(Result::VT_VOTES, votes);
  }
  ResultBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ResultBuilder &operator=(const ResultBuilder &);
  flatbuffers::Offset<Result> Finish() {
    const auto end = fbb_.EndTable(start_, 1);
    auto o = flatbuffers::Offset<Result>(end);
    return o;
  }
};

inline flatbuffers::Offset<Result> CreateResult(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<int64_t>> votes = 0) {
  ResultBuilder builder_(_fbb);
  builder_.add_votes(votes);
  return builder_.Finish();
}

inline flatbuffers::Offset<Result> CreateResultDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<int64_t> *votes = nullptr) {
  return Strawpoll::CreateResult(
      _fbb,
      votes ? _fbb.CreateVector<int64_t>(*votes) : 0);
}

struct Request FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_TYPE = 4,
    VT_VOTE = 6
  };
  RequestType type() const {
    return static_cast<RequestType>(GetField<int8_t>(VT_TYPE, 0));
  }
  int64_t vote() const {
    return GetField<int64_t>(VT_VOTE, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_TYPE) &&
           VerifyField<int64_t>(verifier, VT_VOTE) &&
           verifier.EndTable();
  }
};

struct RequestBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(RequestType type) {
    fbb_.AddElement<int8_t>(Request::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  void add_vote(int64_t vote) {
    fbb_.AddElement<int64_t>(Request::VT_VOTE, vote, 0);
  }
  RequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RequestBuilder &operator=(const RequestBuilder &);
  flatbuffers::Offset<Request> Finish() {
    const auto end = fbb_.EndTable(start_, 2);
    auto o = flatbuffers::Offset<Request>(end);
    return o;
  }
};

inline flatbuffers::Offset<Request> CreateRequest(
    flatbuffers::FlatBufferBuilder &_fbb,
    RequestType type = RequestType_Poll,
    int64_t vote = 0) {
  RequestBuilder builder_(_fbb);
  builder_.add_vote(vote);
  builder_.add_type(type);
  return builder_.Finish();
}

struct Response FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_TYPE = 4,
    VT_POLL = 6,
    VT_RESULT = 8,
    VT_ERROR = 10
  };
  ResponseType type() const {
    return static_cast<ResponseType>(GetField<int8_t>(VT_TYPE, 0));
  }
  const Poll *poll() const {
    return GetPointer<const Poll *>(VT_POLL);
  }
  const Result *result() const {
    return GetPointer<const Result *>(VT_RESULT);
  }
  const flatbuffers::String *error() const {
    return GetPointer<const flatbuffers::String *>(VT_ERROR);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_TYPE) &&
           VerifyOffset(verifier, VT_POLL) &&
           verifier.VerifyTable(poll()) &&
           VerifyOffset(verifier, VT_RESULT) &&
           verifier.VerifyTable(result()) &&
           VerifyOffset(verifier, VT_ERROR) &&
           verifier.Verify(error()) &&
           verifier.EndTable();
  }
};

struct ResponseBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(ResponseType type) {
    fbb_.AddElement<int8_t>(Response::VT_TYPE, static_cast<int8_t>(type), 0);
  }
  void add_poll(flatbuffers::Offset<Poll> poll) {
    fbb_.AddOffset(Response::VT_POLL, poll);
  }
  void add_result(flatbuffers::Offset<Result> result) {
    fbb_.AddOffset(Response::VT_RESULT, result);
  }
  void add_error(flatbuffers::Offset<flatbuffers::String> error) {
    fbb_.AddOffset(Response::VT_ERROR, error);
  }
  ResponseBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ResponseBuilder &operator=(const ResponseBuilder &);
  flatbuffers::Offset<Response> Finish() {
    const auto end = fbb_.EndTable(start_, 4);
    auto o = flatbuffers::Offset<Response>(end);
    return o;
  }
};

inline flatbuffers::Offset<Response> CreateResponse(
    flatbuffers::FlatBufferBuilder &_fbb,
    ResponseType type = ResponseType_Poll,
    flatbuffers::Offset<Poll> poll = 0,
    flatbuffers::Offset<Result> result = 0,
    flatbuffers::Offset<flatbuffers::String> error = 0) {
  ResponseBuilder builder_(_fbb);
  builder_.add_error(error);
  builder_.add_result(result);
  builder_.add_poll(poll);
  builder_.add_type(type);
  return builder_.Finish();
}

inline flatbuffers::Offset<Response> CreateResponseDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    ResponseType type = ResponseType_Poll,
    flatbuffers::Offset<Poll> poll = 0,
    flatbuffers::Offset<Result> result = 0,
    const char *error = nullptr) {
  return Strawpoll::CreateResponse(
      _fbb,
      type,
      poll,
      result,
      error ? _fbb.CreateString(error) : 0);
}

}  // namespace Strawpoll

#endif  // FLATBUFFERS_GENERATED_STRAWPOLL_STRAWPOLL_H_
