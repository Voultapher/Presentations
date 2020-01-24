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
  using ptr_t = const uint8_t*;

  ptr_t data;
  const size_t size;

  constexpr FlatBufferRef(ptr_t d, size_t s) noexcept : data(d), size(s) {}
};

#include <iostream> // tmp debug
class FlatBufferWrapper
{
public:
  template<
    typename Func,
    typename = std::enable_if_t<
      !std::is_same_v<std::decay_t<Func>, FlatBufferWrapper>
    >
  > explicit FlatBufferWrapper(Func&& func)
  {
    flatbuffers::FlatBufferBuilder builder;
    func(builder);
    buffer_ = builder.Release();
  }

  FlatBufferWrapper(const FlatBufferWrapper&) = delete;
  FlatBufferWrapper(FlatBufferWrapper&&) = default;

  FlatBufferWrapper& operator= (const FlatBufferWrapper&) = delete;
  FlatBufferWrapper& operator= (FlatBufferWrapper&& other) = default;

  ~FlatBufferWrapper() = default;

  constexpr FlatBufferRef ref() const noexcept
  {
    return { buffer_.data(), buffer_.size() };
  }

  // copy new buffer into same area as previous
  // this way we should never invalidate result_
  void inplace_assign(const FlatBufferWrapper& other)
  {
    if (other.buffer_.size() != buffer_.size())
    {
      std::cerr << "inplace_assign() size mismatch!\n";
      return;
    }

    std::copy_n(other.buffer_.data(), other.buffer_.size(), buffer_.data());
  }

private:
  flatbuffers::DetachedBuffer buffer_;
};

template<typename T, size_t N> FlatBufferWrapper make_msg(T (&msg)[N])
{
  return FlatBufferWrapper{
    [&msg](flatbuffers::FlatBufferBuilder& builder) -> void
    {
      const auto error = builder.CreateString(msg);

      Strawpoll::ResponseBuilder res(builder);
      res.add_type(Strawpoll::ResponseType_Error);
      res.add_error(error);
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

template<typename T> class PollData : public PollDetail
{
public:
  using vote_guard_t = T;
  vote_guard_t vote_guard;

  explicit PollData() : votes_{}, result_{make_result(votes_)} {}

  PollData(const PollData&) = delete;
  PollData(PollData&&) = default;

  PollData& operator= (const PollData&) = delete;
  PollData& operator= (PollData&&) = default;

  ~PollData() = default;

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

  FlatBufferRef result_ref() const noexcept
  {
    return result_.ref();
  }

  template<typename FF, typename SF> void register_vote(
    const vote_t vote,
    const typename vote_guard_t::address_t& address,
    FF&& fail_func,
    SF&& success_func
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
      fail_func(result_ref());
      return;
    }

    ++votes_[vote];
    result_.inplace_assign(make_result(votes_));
    success_func(result_ref());
  }

private:
  votes_t votes_;
  FlatBufferWrapper result_;
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
template<typename T> struct HippieVoteGuard
{
  using address_t = T;

  bool register_address(const address_t&) { return true; }
  bool has_voted(const address_t&) { return false; }
};
