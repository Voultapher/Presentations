#pragma once

#include <type_traits>
#include <utility>

#include <memory>

#include <vtable.hpp>

namespace inline
{

template<typename> class function;

template<typename R, typename... Args>
class function<R(Args...)>
{
public:
  static constexpr size_t size = sizeof(void(*)) * 2;
  using storage_t = std::aligned_storage_t<size>;
  using vtable_t = detail::vtable<R, Args...>;
  using vtable_ptr_t = const vtable_t*;

  function() noexcept :
    vtable_ptr_{detail::empty_vtable_ptr<R, Args...>}
  {}

  template<
    typename T,
    typename C = std::decay_t<T>,
    typename = std::enable_if_t<!std::is_same<C, function>::value>
  > function(T&& closure)
  {
    static_assert(std::is_copy_constructible<T>::value,
			"constructing function with move only type is invalid!");

    static_assert(sizeof(C) <= size,
      "inplace function closure too large");

    // TODO module alignment check
    static_assert(std::alignment_of<C>::value <=
      std::alignment_of<storage_t>::value,
      "inplace function closure alignment too large");

    static const vtable_t vt{detail::wrapper<C>{}};
    vtable_ptr_ = std::addressof(vt);

    new (std::addressof(storage_)) C{std::forward<C>(closure)};
  }

  function(const function& other) :
    vtable_ptr_{other.vtable_ptr_}
  {
    vtable_ptr_->copy_ptr(
      std::addressof(storage_),
      std::addressof(other.storage_)
    );
  }

  function(function&& other) :
    vtable_ptr_{other.vtable_ptr_},
    storage_{other.storage_}
  {
    other.vtable_ptr_ = detail::empty_vtable_ptr<R, Args...>;
  }

  function& operator= (const function& other)
  {
    if (this != std::addressof(other))
    {
      vtable_ptr_->destructor_ptr(std::addressof(storage_));

      vtable_ptr_ = other.vtable_ptr_;
      vtable_ptr_->copy_ptr(
        std::addressof(storage_),
        std::addressof(other.storage_)
      );
    }
    return *this;
  }

  function& operator= (function&& other)
  {
    if (this != std::addressof(other))
    {
      vtable_ptr_->destructor_ptr(std::addressof(storage_));

      vtable_ptr_ = other.vtable_ptr_;
      storage_ = other.storage_;

      other.vtable_ptr_ = detail::empty_vtable_ptr<R, Args...>;
    }
    return *this;
  }

  ~function()
  {
    vtable_ptr_->destructor_ptr(std::addressof(storage_));
  }

  R operator() (Args... args) const
  {
    return vtable_ptr_->invoke_ptr(
      std::addressof(storage_),
      std::forward<Args>(args)...
    );
  }

private:
  vtable_ptr_t vtable_ptr_;
  mutable storage_t storage_;
};

} // namespace inline
