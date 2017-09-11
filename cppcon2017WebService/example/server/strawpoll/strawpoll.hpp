#pragma once

#include <array>
#include <cstdint>

#include <vector>
#include <unordered_set>
//#include <string>

#include <strawpoll_generated.h>

int foo() {
 return 3;
}

struct FlatBufferRef
{
  // should be invoke_result
  using ptr_t = decltype(
    std::declval<flatbuffers::FlatBufferBuilder>().GetBufferPointer()
  );

  ptr_t data;
  size_t size;

  constexpr FlatBufferRef(ptr_t d, size_t s) noexcept : data(d), size(s) {}
};

class FlatBufferWrapper
{
public:
  template<typename Func> explicit FlatBufferWrapper(Func&& func)
    : builder_(1024)
  {
    func(builder_);
  }

  FlatBufferWrapper(const FlatBufferWrapper&) = default;
  FlatBufferWrapper(FlatBufferWrapper&&) = default;

  FlatBufferWrapper& operator=(const FlatBufferWrapper&) = default;
  FlatBufferWrapper& operator=(FlatBufferWrapper&&) = default;

  ~FlatBufferWrapper() = default;

  constexpr FlatBufferRef ref() const noexcept
  {
    return { builder_.GetBufferPointer(), builder_.GetSize() };
  }

private:
  flatbuffers::FlatBufferBuilder builder_;
};

template<typename T, size_t N> FlatBufferWrapper make_msg(T (&msg)[N])
{
  return FlatBufferWrapper{
    [&msg](flatbuffers::FlatBufferBuilder& builder) -> void
    {
      Strawpoll::ResponseBuilder res(builder);
      res.add_type(Strawpoll::ResponseType_Error);
      res.add_error(builder.CreateString(msg));
      builder.Finish(res.Finish());
    }
  };
}

struct PollDetail
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
};

FlatBufferWrapper make_result(PollDetail::votes_t& votes)
{
  return FlatBufferWrapper{
    [&votes](flatbuffers::FlatBufferBuilder& builder) -> void
    {
      const auto result = Strawpoll::CreateResult(
        builder,
        builder.CreateVector(votes.data(), votes.size())
      );

      Strawpoll::ResponseBuilder res(builder);
      res.add_type(Strawpoll::ResponseType_Result);
      res.add_result(result);
      builder.Finish(res.Finish());
    }
  };
}

template<typename T> struct PollData : public PollDetail
{
  votes_t votes{};

  using vote_guard_t = T;
  vote_guard_t vote_guard;

  const FlatBufferWrapper poll_response{
    [&title = title, &options = options](
      flatbuffers::FlatBufferBuilder& builder
    ) -> void
    {
      const auto poll = Strawpoll::CreatePoll(
        builder,
        builder.CreateString(title),
        builder.CreateVectorOfStrings([&options]{
          std::vector<std::string> ret;
          for (const auto& option : options)
            ret.emplace_back(option);
          return ret;
        }())
      );

      Strawpoll::ResponseBuilder res(builder);
      res.add_type(Strawpoll::ResponseType_Poll);
      res.add_poll(poll);

      builder.Finish(res.Finish());
    }
  };

  struct ErrorResponses {
    const FlatBufferWrapper invalid_message = make_msg("Invalid request message");
    const FlatBufferWrapper invalid_type = make_msg("Invalid request type");
    const FlatBufferWrapper invalid_vote = make_msg("Invalid vote");
  };
  ErrorResponses error_responses{};

  template<typename FailFunc, typename SuccessFunc> void register_vote(
    const vote_t vote,
    const typename vote_guard_t::address_t& address,
    FailFunc&& fail_func,
    SuccessFunc&& success_func
  )
  {
    if (
      vote < 0
      || vote > static_cast<PollDetail::vote_t>(PollDetail::options.size())
    ) {
      fail_func(error_responses.invalid_vote.ref());
      return;
    }

    if (!vote_guard.register_address(address))
    {
      fail_func(make_result(votes).ref());
      return;
    }

    ++votes[vote];
    success_func(make_result(votes).ref());
  }
};

template<typename T, template <typename> class H> class VoteGuard
{
public:
  using address_t = T;
  using hash_t = H<address_t>;
  using set_t = std::unordered_set<address_t, hash_t>;

  VoteGuard() = default;

  VoteGuard(const VoteGuard&) = default;
  VoteGuard(VoteGuard&&) = default;

  VoteGuard& operator= (const VoteGuard&) = default;
  VoteGuard& operator= (VoteGuard&&) = default;

  ~VoteGuard() = default;

  bool register_address(const address_t& address)
  {
    return set_.insert(address).second;
  }

  bool has_voted(const address_t& address)
  {
    return set_.count(address) != 0;
  }

private:
  set_t set_;
};

struct EmptyAddress {};
struct HippieVoteGuard
{
  using address_t = EmptyAddress;

  bool register_address(const address_t&) { return true; }
  bool has_voted(const address_t&) { return false; }
};
