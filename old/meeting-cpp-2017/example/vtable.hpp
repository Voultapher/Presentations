#pragma once

#include <memory>
#include <utility>
#include <functional>

namespace detail
{
template<typename T> struct wrapper
{
  using type = T;
};

template<typename R, typename... Args> struct vtable
{
  using storage_ptr_t = void*;

  using invoke_ptr_t = R(*)(storage_ptr_t, Args&&...);
	using copy_ptr_t = void(*)(storage_ptr_t, storage_ptr_t);
	using destructor_ptr_t = void(*)(storage_ptr_t);

  const invoke_ptr_t invoke_ptr;
  const copy_ptr_t copy_ptr;
  const destructor_ptr_t destructor_ptr;
  const size_t size;

  explicit constexpr vtable() noexcept :
    invoke_ptr{ static_cast<invoke_ptr_t>(
      [](storage_ptr_t, Args&&...) -> R
      { throw std::bad_function_call(); }
    )},
    copy_ptr{ static_cast<copy_ptr_t>(
      [](storage_ptr_t, storage_ptr_t) noexcept -> void {}
    )},
    destructor_ptr{static_cast<destructor_ptr_t>(
      [](storage_ptr_t) noexcept -> void {}
    )},
    size{}
  {}

  template<typename C> explicit constexpr vtable(wrapper<C>) noexcept :
    invoke_ptr{ static_cast<invoke_ptr_t>(
      [](storage_ptr_t storage_ptr, Args&&... args) -> R
			{ return (*static_cast<C*>(storage_ptr))(
        std::forward<Args>(args)...
      ); }
		)},
		copy_ptr{ static_cast<copy_ptr_t>(
      [](storage_ptr_t dst_ptr, storage_ptr_t src_ptr) -> void
		  { new (dst_ptr) C{ (*static_cast<C*>(src_ptr)) }; }
    )},
		destructor_ptr{ static_cast<destructor_ptr_t>(
			[](storage_ptr_t storage_ptr) noexcept -> void
			{ static_cast<C*>(storage_ptr)->~C(); }
		)},
    size{sizeof(C)}
  {}

  vtable(const vtable&) = delete;
  vtable(vtable&&) = delete;

  vtable& operator= (const vtable&) = delete;
  vtable& operator= (vtable&&) = delete;

  ~vtable() = default;
};

template<typename R, typename... Args>
static const vtable<R, Args...> empty_vtable{};

template<typename R, typename... Args>
static const auto empty_vtable_ptr =
std::addressof(empty_vtable<R, Args...>);

} // namespace detail
