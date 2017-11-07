#pragma once

#include <type_traits>
#include <utility>
#include <memory>
#include <functional>

namespace variant
{
static int bar(int arg)
{
  return arg * arg;
}

static int foo(int arg)
{
  return bar(arg + arg);
}

struct trivial_closure_a
{
  int arg;
  trivial_closure_a(const int a) : arg(a) {}

  int operator() () const noexcept
  {
    return foo(arg);
  }
};

struct trivial_closure_b
{
  int arg_a;
  int arg_b;
  trivial_closure_b(const int a, const int b) : arg_a{a}, arg_b{b} {}

  int operator() () const noexcept
  {
    return foo(arg_a + arg_b);
  }
};

#include <vector>
struct complex_closure
{
  std::vector<int> vec;
  complex_closure(std::vector<int> arg) : vec{std::move(arg)} {}

  complex_closure(const complex_closure&) = default;
  complex_closure(complex_closure&&) = default;

  complex_closure& operator= (const complex_closure&) = default;
  complex_closure& operator= (complex_closure&&) = default;

  ~complex_closure() = default;

  int operator() () const noexcept
  {
    return foo(vec.back());
  }
};

template<typename> class function;

template<typename R, typename... Args>
class function<R(Args...)>
{
public:
  using index_t = size_t;
  using storage_t = std::aligned_storage_t<
    sizeof(complex_closure),
    std::alignment_of<complex_closure>::value
  >;

  function() noexcept : index_{3} {}

  function(trivial_closure_a closure) noexcept : index_{0}
  {
    new (std::addressof(storage_)) trivial_closure_a{closure};
  }

  function(complex_closure closure) : index_{1}
  {
    new (std::addressof(storage_)) complex_closure{std::move(closure)};
  }

  function(trivial_closure_b closure) noexcept : index_{2}
  {
    new (std::addressof(storage_)) trivial_closure_b{closure};
  }

  function(const function& other) :
    index_{other.index_}
  {
    switch(index_)
    {
      case 0:
        storage_ = other.storage_; // trivial copy
        break;
      case 1:
        new (std::addressof(storage_)) complex_closure{
          (*static_cast<complex_closure*>(
            static_cast<void*>(std::addressof(other.storage_))
          ))
        };
        break;
      case 2:
        storage_ = other.storage_; // trivial copy
        break;
      default: /* empty assing */ break;
    }
  }

  function(function&& other) :
    index_{other.index_},
    storage_{other.storage_}
  {
    other.index_ = 4;
  }

  function& operator= (const function& other)
  {
    if (this != std::addressof(other))
    {
      index_ = other.index_;
      switch(index_)
      {
        case 0:
          storage_ = other.storage_; // trivial copy
          break;
        case 1:
          new (std::addressof(storage_)) complex_closure{
            (*static_cast<complex_closure*>(
              static_cast<void*>(std::addressof(other.storage_))
            ))
          };
          break;
        case 2:
          storage_ = other.storage_; // trivial copy
          break;
        default: /* empty assing */ break;
      }
    }
    return *this;
  }

  function& operator= (function&& other)
  {
    if (this != std::addressof(other))
    {
      index_ = other.index_;
      storage_ = other.storage_;
      other.index_ = 4;
    }
    return *this;
  }

  ~function()
  {
    switch(index_)
    {
      case 1:
        static_cast<complex_closure*>(
          static_cast<void*>(std::addressof(storage_))
        )->~complex_closure();
        break;
      default: break; // trivial no-op
    }
  }

  R operator() (Args... args) const
  {
    switch(index_)
    {
      case 0: return (*static_cast<trivial_closure_a*>(
        static_cast<void*>(std::addressof(storage_))
      ))(std::forward<Args>(args)...);
      case 1: return (*static_cast<complex_closure*>(
        static_cast<void*>(std::addressof(storage_))
      ))(std::forward<Args>(args)...);
      case 2: return (*static_cast<trivial_closure_b*>(
        static_cast<void*>(std::addressof(storage_))
      ))(std::forward<Args>(args)...);
      default: throw std::bad_function_call();
    }
  }

private:
  index_t index_;
  mutable storage_t storage_;
};

} // namespace variant
