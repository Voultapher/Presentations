#pragma once

#include <type_traits>
#include <utility>

#include <cstdlib>
#include <memory>

#include <vtable.hpp>

namespace heap
{

template<typename> class function;

template<typename R, typename... Args>
class function<R(Args...)>
{
public:
  using vtable_t = detail::vtable<R, Args...>;
  using vtable_ptr_t = const vtable_t*;

  function() noexcept :
    vtable_ptr_{detail::empty_vtable_ptr<R, Args...>},
    storage_ptr_{nullptr}
  {}

  template<
    typename T,
    typename C = std::decay_t<T>,
    typename = std::enable_if_t<!std::is_same<C, function>::value>
  > function(T&& closure) : storage_ptr_{std::malloc(sizeof(C))}
  {
    static_assert(std::is_copy_constructible<T>::value,
			"constructing function with move only type is invalid!");

    static const vtable_t vt{detail::wrapper<C>{}};
    vtable_ptr_ = std::addressof(vt);

    new (storage_ptr_) C{std::forward<C>(closure)};
  }

  function(const function& other) :
    vtable_ptr_{other.vtable_ptr_},
    storage_ptr_{std::malloc(vtable_ptr_->size)}
  {
    vtable_ptr_->copy_ptr(storage_ptr_, other.storage_ptr_);
  }

  function(function&& other) :
    vtable_ptr_{other.vtable_ptr_},
    storage_ptr_{other.storage_ptr_}
  {
    other.vtable_ptr_ = detail::empty_vtable_ptr<R, Args...>;
    other.storage_ptr_ = nullptr;
  }

  function& operator= (const function& other)
  {
    if (this != std::addressof(other))
    {
      destroy();

      vtable_ptr_ = other.vtable_ptr_;
      storage_ptr_ = std::malloc(vtable_ptr_->size);
      vtable_ptr_->copy_ptr(storage_ptr_, other.storage_ptr_);
    }
    return *this;
  }

  function& operator= (function&& other)
  {
    if (this != std::addressof(other))
    {
      destroy();

      vtable_ptr_ = other.vtable_ptr_;
      storage_ptr_ = other.storage_ptr_;

      other.vtable_ptr_ = detail::empty_vtable_ptr<R, Args...>;
    }
    return *this;
  }

  ~function()
  {
    destroy();
  }

  R operator() (Args... args) const
  {
    return vtable_ptr_->invoke_ptr(
      storage_ptr_,
      std::forward<Args>(args)...
    );
  }

private:
  vtable_ptr_t vtable_ptr_;
  mutable typename vtable_t::storage_ptr_t storage_ptr_;

  void destroy()
  {
    if (storage_ptr_)
    {
      vtable_ptr_->destructor_ptr(storage_ptr_);
      std::free(storage_ptr_);
    }
  }
};

} // namespace heap
