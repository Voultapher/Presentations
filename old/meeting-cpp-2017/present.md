### Is
[`std::function`]()
### *really* the best we can do?

Note:
- Is `std::function` really the best we can do? >>

---

# Yes

Note:
- Yes!
- Let me rephrase that.
- Is `std::function` the best **it** can do?
- Yes!
- Is `std::function` the best **you** can do? >>

---

# No

Note:
- No!
- Intro
- If you have questions, just raise your arm.

---

### Roadmap

* Problem
<!-- .element: class="fragment" -->

* Status Quo
<!-- .element: class="fragment" -->

* Alternatives
<!-- .element: class="fragment" -->

Note:
- First we'll try to understand what the problem is. >>
- Then we'll try to implement `std::function`,
to understand the root of those problems. >>
- And finally we'll explore alternatives. >>

---

`std::max`

```cpp
namespace std
{
  template<typename T>
  constexpr const T& max(
    const T& a,
    const T& b,
    std::function<bool(const T&, const T&)> compare
  );
}
```

Note:
- One `std::max` signature
- Wrong?

---

`std::max`

```cpp
namespace std
{
  template<typename T, typename Compare>
  constexpr const T& max(
    const T& a,
    const T& b,
    Compare compare
  );
}
```

Note:
- Yes I lied.
- `compare` is a template constrained parameter.
- Why?
- `std::function` more readable
- `std::function` more constrained
- [Benchmark how much slower?]
(http://quick-bench.com/v0VC_hCf4naIfmjdv5PEpEqP-KQ)
- More than 20x slower, why?
- Note we over constrain so the user has to specify to function type.
- `std::max` can't be used in constexpr context anymore.

---

Github Search

```cpp
template <typename A, typename B>
std::function<B(A)> Y(
  std::function<std::function<B(A)>(std::function<B(A)>)> f
) {
	return [f](A x) {
		return f(Y(f))(x);
	};
}
```
<!-- .element: class="fragment" -->

Note:
- No one would write code like this, right?
- Github search
- Majority was free functions >>
- A Y combinator

---

Github Search

```cpp
template<typename T>
std::vector<T> filter(
  std::function<bool(T)> pred,
  const std::vector<T> & vec
);
```

Note:
- Tried being generic
- Swap the arguments, replace the predicate with a template,
replace the vector with a range and this could be in the STL.

---

Github Search

```cpp
#include <functional>
typedef float TYPE;

std::function<TYPE(TYPE)> Const(TYPE A);

std::function<TYPE(TYPE)> Sum(
  std::function<TYPE(TYPE)> A,
  std::function<TYPE(TYPE)> B
);

[...]
```

Note:
- Lambda calculus
- That's quickly going to become expensive.

---

Github Search

```cpp
std::string join(
  std::list<std::string> entries,
  std::function<std::string(
    std::string,
    std::string
  )> join_lines
);
```

Note:
- Again tried being generic
- Just beautiful
- Imagine calling this with 2 string literals.
- Heap allocate the list nodes, which being strings might allocate again,
the binary predicate might have to allocate it's arguments again,
and let's hope for return value optimization.

---

`std::function` is [*magic*]()

Note:
- Often when I hear people talk about `std::function`,
they treat as if it was *magic*.

---

![alt text](burst_bubble.jpg)

Note:
- Burst bubble
- No *magic*!
- Implement

---

#### Are we doing heap allocation?

```cpp
std::array<int, 4> buf;
std::function<void()> a = [buf]() { return 42; };
```
<!-- .element: class="fragment" -->

```cpp
std::function<void()> a = []() { return 42; };
```
<!-- .element: class="fragment" -->

```cpp
void foo(const std::function<void()>& arg)
{
  const auto a = arg;
}
```
<!-- .element: class="fragment" -->

Note:
- Game >>
- Are we doing heap allocation?
- We don't know, it's implementation defined. >>
- How about now?
- We still don't know, it's implementation defined. >>
- Extra: function pointer is guaranteed SBO, lambda can be casted to,
not sure if applies.
- Here?
- We really can't know here. Depends on the caller. >>

---

# Closure

Note:
- Ok, before we dive deeper into the topic,
lets talk about some concepts.
- Closure.
- A callable thing, that may contain state.
- Most common example. >>

---

Lambda

```cpp
int val = 3;
auto lam = [val](int arg) -> int { return arg + val; };
```

Note:
- Lambdas
- Not new concept

---

C++03 Lambda

```cpp
struct closure
{
    const int val;
    explicit closure(int v) : val(v) {}

    int operator() (int arg)
    {
        return arg + val;
    }
};

int val = 3;
closure old{ val };
```

Note:
- Back before C++11, people had ways of expressing the same.
- Less pretty
- Error prone

---

```cpp
for (int i = 1; i <= 10; ++i)
  std::cout << i << '\n';
```
<!-- .element: class="fragment" -->

Note:
- During the talk, when I show you a small code snippet containing `cout`,
I want you to tell me what you think it prints.
- Let's give it a quick try.

---

### Size of callable *things*

Note:
- Size of callable *things*

---

Function pointers

```cpp
void foo(int arg);

void bar()
{
  auto f = foo;
  std::cout << sizeof(f);
}
```

Usually 8 or 4 byte
<!-- .element: class="fragment" -->

Note:
- Let's start with function pointers
- On most platforms it's the size of a pointer, so 8 or 4 byte respectively

---

Lambda

```cpp
auto f = [](char arg) {};
std::cout << sizeof(f);
```

1 byte
<!-- .element: class="fragment" -->

```cpp
int val = 3;
auto f = [val](int arg) {};
std::cout << sizeof(f);
```

sizeof(int) bytes
<!-- .element: class="fragment" -->

```cpp
auto f = [](char arg[10]) {};
std::cout << sizeof(f);
```

1 byte
<!-- .element: class="fragment" -->

Note:
- Lambdas?
- 1 byte, so we already see a difference
- This time it's `sizeof(int)`, usually 4 byte
- Again, 1 byte.
- The size of a closure depends only on the size of what it captures.

---

### Function pointer types

Note:
- Function pointers
- Arcane syntax

---

Free Function

```cpp
void foo(int arg);
auto f = foo;
```

```cpp
static_assert(std::is_same_v<decltype(f), void(*)(int)>);
```
<!-- .element: class="fragment" -->

Note:
- What do you think the type of f is?
- Still fairly reasonable

---

Member function pointers

```cpp
class foo
{
  void bar(int arg);
};

auto f = &foo::bar;
```
<!-- .element: class="fragment" -->

Note:
- How about member functions
- Bar is private

---

Member function pointer

```cpp
class foo
{
public:
  void bar(int arg);
};

auto f = &foo::bar;
```

```cpp
static_assert(std::is_same_v<decltype(f), void(foo::*)(int)>);

```
<!-- .element: class="fragment" -->

```
foo obj;
(obj.*f)(3);
```
<!-- .element: class="fragment" -->

Note:
- Type of f
- Calling f
- Not `f(3)`
- Makes sense, member function pointer does not represent state.
- Only offset into object

---

Lambda

```cpp
auto f = [](int arg) {};
```

```cpp
static_assert(std::is_same_v<decltype(f), decltype(f)>);
```
<!-- .element: class="fragment" -->

Note:
- How about this?
- I admit, that was a trick question.
- Every lambda has a unique type.

---

Lambda -> Function Pointer

```
void(*f)(int) = [](int arg) { std::cout << arg; };

f(3);
```

Note:
- Lambdas are implicitly convertible to function pointers.

---

`naive::function`

```
class function
{
public:
  using invoke_ptr_t = void(*)(int);

  explicit function(invoke_ptr_t f) : invoke_ptr_(f) {}

  void operator() (int arg) const
  {
    invoke_ptr_(arg);
  }
private:
  invoke_ptr_t invoke_ptr_;
};
```

Note:
- Naive function closure implementation.
- Ignoring rule of 6 to make it more readable.
- The `invoke_ptr_` is a function pointer, initialized by the constructor.
- The call operator takes the function argument,
and invokes the function pointer with the argument.

---

Example

```cpp
function f{[](int a) { std::cout << a + 2; }};
f(5);
```

Note:
- Now possible.

---

Generic

```
template<typename> class function;

template<typename R, typename... Args>
class function<R(Args...)>
```

```cpp
{
public:
  using invoke_ptr_t = R(*)(Args...);

  explicit function(invoke_ptr_t f) : invoke_ptr_(f) {}

  R operator() (Args... args) const
  {
    return invoke_ptr_(std::forward<Args>(args)...);
  }
private:
  invoke_ptr_t invoke_ptr_;
};
```
<!-- .element: class="fragment" -->

Note:
- Make generic
- `std::function` class signature
- `invoke_ptr_t`
- Call operator arguments by value and const.

---

Usage

```cpp
function<int(int)> f{[](int a) { return a + 3; }};

std::cout << f(5);
```

Note:
- Example usage
- What are we missing?

---

Closure Storage

```cpp
const int b = 3;
function<int(int)> f{[b](int a) { return a + b; }};

const int res = f(5);
```

Note:
- What will happen?

---

Compiler Error

```
[...]

no known conversion from
'(lambda at /path/example.cpp:26:5)'
to
'const function<int (int)>'

[...]

1 error generated.
Compiler exited with result code 1
```

Note:
- Only lambdas without capture list convertible to function pointer.
- Kinda makes sense.
- Let's say we capture an object with the size of 20 bytes,
the function pointer has only 8 bytes of storage.

---

### Convert to functional programming?
<!-- .element: class="fragment" -->

Note:
- The solution is simple
- Only use pure functions

---

# Static
<!-- .element: class="fragment" -->

Note:
- Maybe, there is a way of constructing a function pointer
with a capturing lambda.
- The issue is, that we have to store the closure somewhere.
- Enter C++'s least ambiguous keyword.

---

Implicit Lambda Capture

```cpp
static int val = 4;
auto pure = [](int arg)
{
  return arg + val;
};
std::cout << pure(-1);
```

```
int(*f)(int) = pure;
```
<!-- .element: class="fragment" -->

Note:
- Pure lambdas not really pure
- Compiler can resolve the address of val at compile time.
- Now we can do.

---

Implicit Lambda Capture

```
const int val = 4;
auto pure = [](int arg)
{
  return arg + val;
};
std::cout << pure(-1);
```

Note:
- Something curious I found out just recently.
- Which Error?
- Valid C++11
- Cppreference: "A variable can be used without being captured if it does not have automatic storage duration (i.e. it is not a local variable or it is static or thread local) or if it is not odr-used in the body of the lambda."
- Standard 6.2.3: "A variable x whose name appears as a potentially-evaluated expression ex is odr-used by ex unless applying the
lvalue-to-rvalue conversion (7.1) to x yields a constant expression ..."
- Types are also visible inside a lambda.
- With that we can add a new constructor to our `function` class.

---

Static Closure Storage

```cpp
static T capture{ std::forward<T>(closure) };

invoke_ptr_ = static_cast<invoke_ptr_t>([](Args... args) -> R
{
  return capture(std::forward<Args>(args)...);
});
```

Note:
- We are golden now, right?
- Someone see any issues?
- We haven't talked about copying and moving, but let's do that later.
- We are doing improper argument type forwarding.
- To avoid unnecessary moving, it should be >>

---

Argument Type Forwarding

```cpp
static T capture{ std::forward<T>(closure) };

invoke_ptr_ = static_cast<invoke_ptr_t>([](Args&&... args) -> R
{
  return cap(std::forward<Args>(args)...);
});
```
Note:
- We should be passing in the arguments as rvalue references.
- But that does not work, as that would not fit the function pointer type. >>

---

Example

```cpp
using func_t = function<int(void)>;

std::vector<func_t> vec;

for (int i = 0; i <= 3; ++i)
  vec.emplace_back([i]() { return i; });

std::cout << vec.back()();
```
<!-- .element: class="fragment" -->

```cpp
static T capture{ std::forward<T>(closure) };
```
<!-- .element: class="fragment" -->
Note:
- Let's look at this
- You guessed it, it pirnts 0.
- Although each lambda has a unique type, here there is only **ONE**
lambda, which means that, >>
- `capture` gets created only once.
- The first time this constructor is called.
---

### Interface Usage

* Easy to use correctly
<!-- .element: class="fragment" -->

* Easy to use incorrectly
<!-- .element: class="fragment" -->

Note:
- Sure, we could tell the user to never do that.
- However this would result in an interface that is:
- Easy to use correctly >> and >> Easy to use incorrectly
- When truly what we want is an interface, >>
---

### Interface Usage

* Easy to use correctly

* Hard to use incorrectly
<!-- .element: class="fragment" -->

Note:
- that is, easy to use correctly >> and >> Hard to use incorrectly.

---

### Storing the Closure

* Class: memory layout
<!-- .element: class="fragment" -->

* Constructor: memory initialization
<!-- .element: class="fragment" -->

* Run time: mutate memory
<!-- .element: class="fragment" -->

Note:
- Before we try figuring out how to store the closure object,
we have to understand the conceptual memory time line. >>
- Class level: here we decide the memory layout. >>
- Constructor level: here we decide how the memory layout is initialized.
Note this still happens at compile time, however the constructor can
no longer change the memory layout.
- The memory layout is strongest way for the compiler to reason about
your program, with that comes the biggest optimization potential. >>
- Run time: here we potentially mutate the memory.

---

Dynamic Closure Storage

```cpp
template<typename C>
explicit constexpr vtable(wrapper<C>) noexcept :
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
  [...]
{}
```
<!-- .element: class="fragment" -->

Note:
- We don't know the closure object size at class level.
- This information only becomes availible at the Constructor level,
it's already to late to change the class layout.
- Classical type erasure
- Avoid inheritance
- More explicit vtable implementation
- Easy
- Let's look at it one piece at a time.

---

Custom vtable

```
template<typename R, typename... Args> struct vtable
{
  using storage_ptr_t = void*;

  using invoke_ptr_t = R(*)(storage_ptr_t, Args&&...);
  using copy_ptr_t = void(*)(storage_ptr_t, storage_ptr_t);
  using destructor_ptr_t = void(*)(storage_ptr_t);

  [...]
}
```

Note:
- 3 core function pointers
- invoke
- copy
- destroy

---

Custom vtable

```cpp
const invoke_ptr_t invoke_ptr;
const copy_ptr_t copy_ptr;
const destructor_ptr_t destructor_ptr;
const size_t size;
```

Note:
- Store one of each and a size

---

Custom vtable

```cpp
template<typename C>
explicit constexpr vtable(wrapper<C>) noexcept :
```

Note:
- Constructor signature
- We only care about the closure type, not the object itself.
- `wrapper<C>` is an empty `struct` around the type,
given that we can't specify constructor template types.

---

Custom vtable

```cpp
invoke_ptr{ static_cast<invoke_ptr_t>(
  [](storage_ptr_t storage_ptr, Args&&... args) -> R
	{
    return (*static_cast<C*>(storage_ptr))(
      std::forward<Args>(args)...
    );
  }
)},
```

Note:
- Initialize `invoke_ptr`
- `storage_ptr_t` is `void*`
- Create function pointer from lambda,
that takes a pointer to the type erased storage object,
and casts it to our closure type.
- Closure type is available in the constructor

---

Custom vtable

```cpp
copy_ptr{ static_cast<copy_ptr_t>(
  [](storage_ptr_t dst_ptr, storage_ptr_t src_ptr) -> void
  {
    new (dst_ptr) C{ (*static_cast<C*>(src_ptr)) };
  }
)},
```

Note:
- Initialize `copy_ptr`
- Placement new to construct into `aligned_storage_t` or `heap memory`

---

Custom vtable

```cpp
destructor_ptr{ static_cast<destructor_ptr_t>(
  [](storage_ptr_t storage_ptr) noexcept -> void
  {
    static_cast<C*>(storage_ptr)->~C();
  }
)},
size{sizeof(C)}
```

Note:
- Initialize `destructor_ptr`
- Call object destructor stored in the memory associated with `storage_ptr`
- Store closure size

---

Dynamic Closure Storage

```cpp
private:
  vtable_ptr_t vtable_ptr_;
  mutable typename vtable_t::storage_ptr_t storage_ptr_;
```

Note:
- `function` class layout
- Pointer to vtable
- and `mutable` `storage_ptr_`, remember, call operator has to be const.

---

Dynamic Closure Storage

```cpp
template<
  typename T,
  typename C = std::decay_t<T>,
> function(T&& closure)
{
  static const vtable_t vt{detail::wrapper<C>{}};
  vtable_ptr_ = std::addressof(vt);

  storage_ptr_ = std::malloc(sizeof(C));
  new (storage_ptr_) C{std::forward<C>(closure)};
}
```

Note:
- For each closure type specific instantiation create `vtable`.
- `vtable_ptr` as link to the `vtable`
- We don't know the size of the closure at type layout.
- So we have to store it on the heap.
- That means calling `malloc` with a certain size.
- Again placement new.
- Note that we have to decay the closure parameter type.

---

Dynamic Closure Storage

```cpp
~function()
{
  if (storage_ptr_)
  {
    vtable_ptr_->destructor_ptr(storage_ptr_);
    std::free(storage_ptr_);
  }
}
```

Note:
- Destructor has to destroy object
- Free object heap memory
- Note, pointers created by `malloc` can be deallocated
without knowing the object type.
- Skip copy ctor, move ctor, copy assignment and move assignment.
- [Compate to `std::function`](http://quick-bench.com/ArL8ERJJwk0vvwqpCYjkmJFOuMo)
- Why so much slower?

---

# SBO

Note:
- Small Buffer Optimization
- At the type layout stage
- Piece of memory to store small objects,
inside the function object on the stack.
- The `int` we capture was small enough,
so `std::function` didn't need to do heap allocation.
- `std::function` and `std::string` do this.

---

Inplace Closure Storage

```py
template<typename R, typename... Args>
class function<R(Args...)>
{
public:
  using storage_t = ???
}
```
<!-- .element: class="fragment" -->

Note:
- Now what should the local variable type be? >>
- It seems we may have to resort to the dark art of type less storage >>
---

`void*`
<!-- .element: class="fragment" -->

Note:
- The type less storage type most of you know is >>
- But we want to avoid heap allocation.
- As luck would have it, even here the deep pockets of C++11 got us covered >>

---

Inplace Closure Storage

`std::aligned_storage`

```cpp
template<
  std::size_t Len,
  std::size_t Align /* default alignment not implemented */
>
struct aligned_storage {
    struct type {
        alignas(Align) unsigned char data[Len];
    };
};
```
<!-- .element: class="fragment" -->

Note:
- It is a byte array, with specific size and alignment >>

---

Inplace Closure Storage

```cpp
{
  std::aligned_storage<
    sizeof(int),
    alignof(int)
  > storage;

  new(&storage)int{ 3 };

  std::cout << static_cast<int&>(storage);
}
std::cout << 6;
```

Note:
- So what does this print?

---

# UB

Note:
- Hmm.. >>

---

Inplace Closure Storage

```cpp
{
  std::aligned_storage<
    sizeof(int),
    alignof(int)
  > storage;

  new(&storage)int{ 3 };

  std::cout << static_cast<int&>(storage);
}
std::cout << 6;
```

Note:
- Yes it prints 3, and then it corrupts the stack trying to leave
the inner scope.
- Who can spot the bug? >>

---

Inplace Closure Storage

```cpp
{
  std::aligned_storage<
    sizeof(int),
    alignof(int)
  >::type storage;

  new(&storage)int{ 3 };

  std::cout << static_cast<int&>(storage);
}
std::cout << 6;
```


Note:
- We forgot a ::type at the end of aligned_storage.
- Trust me you don't want to debug, that.
- By writing into storage, which essentially was an empty struct,
we corrupted the stack.

---

Inplace Closure Storage

```cpp
new (std::addressof(storage_)) C{std::forward<C>(closure)};
```

Note:
- Placement new into our storage object
- Make sure size and alignment fit

---

Inplace Closure Storage

```cpp
R operator() (Args... args) const
{
  return vtable_ptr_->invoke_ptr(
    std::addressof(storage_),
    std::forward<Args>(args)...
  );
}
```

Note:
- Call operator
- Replace `storage_ptr` with the address of the `storage` object

---

Inplace Closure Storage

```cpp
~function()
{
  vtable_ptr_->destructor_ptr(std::addressof(storage_));
}
```

Note:
- Destroy the object inside `storage`.
- Empty `vtable` `destructor_ptr` is no op.
- Note, that we don't have heap memory anymore,
we don't need to free `storage`.
- Again we'll skip copy and move.

---

# SBO

Note:
- Small Buffer Optimization means we store the closure inplace,
if it is small enough like we've just seen.
- Otherwise we do heap allocation.
- [inplace vs `std::function`](http://quick-bench.com/DYFVhe010StaXo3dMmQRKVr0VKQ)
- Faster because less branching
- [Price of loosing inlining](http://quick-bench.com/nfVtNp9sWilZTossqx178p336X8)
- All this abstraction is expensive.
- Note, here the vector reserve is an allocation, so that's part of the cost.
- The `std::function` object should be using SBO, only storing one int.
- Someone else comes along, and adds vec to the capture list.
- 2500x more expensive.
- Note, PGO can help optimize deferred calls.
- If you can know your memory layout, please tell the compiler.

---

Inplace Closure Storage Size

```
static constexpr size_t size = sizeof(void(*)) * 2;
using storage_t = std::aligned_storage_t<size>;
```

Note:
- Make inplace size and alignment configurable,
via non type template parameters of the class signature with defaults.
- What should those defaults look like?
- A function object only makes sense,
if you have to store it inside a container.
- So increasing it's size will hurt memory usage and iteration speed.
- The 3 most common sizes for the resulting object are 32, 48, and 64 bytes,
in line with common x86 cache line size.

---

### Future

```cpp
stdext::inplace_function<
  signature,
  size = default,
  alignment = default
>
```
<!-- .element: class="fragment" -->

```cpp
dyno::function<
  signature,
  storage_policy = default
>
```
<!-- .element: class="fragment" -->

Note:
- Future >>
- SG14, has a proposal for an `inplace_function`,
that never does heap allocation. >>
- Louis Dion proposes a more generic abstraction of storage policies.
- SBO and inplace are part of this bigger set.
- If you want to learn more, watch his CppCon talk from this year.

---

### Variant Closure Storage

* Avoid unused space or heap allocation
<!-- .element: class="fragment" -->

* Potentially perfect in-lining
<!-- .element: class="fragment" -->

Note:
- Crazy, assume we have reflection and constexpr blocks.
- Via reflection or another mechanism,
we accumulate all the types our function object is being instantiate with.
- Then store the closure inside a variant like.
- We know what kind of layout our object can have.
- Just not which one it is at run time. >>
- Only use as much space as we need.
- No heap allocation. >>
- The `static_cast` isn't hidden behind a function pointer.
- The compiler can inline it like any regular function call.

---

Closure Types

```cpp
struct trivial_closure_a
{
  int arg;

  trivial_closure_a(const int a) noexcept : arg(a) {}

  int operator() () const noexcept
  {
    return foo(arg);
  }
};
```

Note:
- Let's assume this is the first type our user instantiates `function` with.

---

Closure Types

```cpp
struct complex_closure
{
  std::vector<int> vec;

  complex_closure(std::vector<int> arg) :
    vec{std::move(arg)}
  {}

  int operator() () const noexcept
  {
    return foo(vec.back());
  }
};
```

Note:
- Second type our user instantiates `function` with.

---

Closure Types

```cpp
struct trivial_closure_b
{
  int arg_a;
  int arg_b;

  trivial_closure_b(const int a, const int b) noexcept :
    arg_a{a}, arg_b{b}
  {}

  int operator() () const noexcept
  {
    return foo(arg_a + arg_b);
  }
};
```

Note:
- Third type our user instantiates `function` with.
- Very similar to our first type.

---

Variant Closure Storage

```cpp
function() noexcept : index_{3} {}
```

```cpp
function(trivial_closure_a closure) noexcept : index_{0}
{
  new (std::addressof(storage_)) trivial_closure_a{closure};
}
```
<!-- .element: class="fragment" -->

Note:
- We keep track of the which type is currently active via an index variable.
- Empty construction, setting the index to one above the last viable index.
- We have 3 types, so 0, 1, and 2 are used.
- Now that we know each used type we can generate a matching constructor. >>
- `trivial_closure_a` first type constructor.
- I'll skip perfect forwarding.

---

Variant Closure Storage

```cpp
function(complex_closure closure) : index_{1}
{
  new (std::addressof(storage_)) complex_closure{
    std::move(closure)
  };
}

function(trivial_closure_b closure) noexcept : index_{2}
{
  new (std::addressof(storage_)) trivial_closure_b{closure};
}
```

Note:
- The constructors for type 2 and 3 are very similar.

---

Variant Closure Storage

```cpp
R operator() (Args... args) const
{
  switch(index_)
  {
    case 0: return (*static_cast<trivial_closure_a*>(
      static_cast<void*>(std::addressof(storage_))
    ))(std::forward<Args>(args)...);
    [...]
    default: throw std::bad_function_call();
  }
}
```

Note:
- When invoking our closure, we switch based on the index,
cast the `storage_` object to the according type and call it.
- The only indirection is the switch.
- Inlining should be very transparent to the optimizer.

---

Variant Closure Storage

```cpp
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
```

Note:
- In the destructor we can make use of the fact that type 1 and 3 are
trivially destructible. So we don't need to do anything.
- Should the stored object be the `complex_closure` we call it's destructor.
- Again we'll skip explaining the copy and move.
- [Benchmark](http://quick-bench.com/RcG3GqFEDjjW7BLoGiHV6hP9q2s)
- What could possible suffer with this approach?
- Growing the type set might trash instruction cash.

---

Move Only Closure

```cpp
del_t del_a = rand_bool
  ? copy_move_closure{}
  : move_only_closure{};

del_t del_b = del_a; // can we copy?
```
<!-- .element: class="fragment" -->

Note:
- Some people are curious as to why we can't construct `std::function`,
with a move only type.
- Consider this example. >>
- We would have to make decisions at compile time, about run time properties.
- I see 2 possible solutions,
if you *really* want to build `function` objects with move only types.
- You could either make a move only `function`, or have a throwing copy. >>

---

### Lessons Learned

* Free functions -> `template<typename F>`
<!-- .element: class="fragment" -->

* [`std::function`]() is an opaque abstraction!
<!-- .element: class="fragment" -->

* The price of *magic* is runtime!
<!-- .element: class="fragment" -->

Note:
- Lessons Learned >>
- If it's a free function,
using templates is basically always a better option. >>
- Note how I always say can, not will. Using run time polymorphism,
will make your performance much less predictable.
- Remember the earlier example, someone adds a single value to a capture,
and suddenly we are doing recursive allocation.
- `std::function` is an opaque abstraction!
- Personally, I try to avoid `std::function` whenever possible.
- I think it is a bad abstraction most of the time.
- It only make sense in very specific library level use cases.
- Function signature issue, usually over constraining. >>
- The price of *magic* is runtime.
- Interfaces that try to guess what the user wants are fundamentally doomed.

---

When should you use [`std::function`]()?

* Has to be stored inside a container
<!-- .element: class="fragment" -->

* You really can't know the layout at compile time
<!-- .element: class="fragment" -->

* You **really** can't know the layout at compile time
<!-- .element: class="fragment" -->

* **All** your closure types are move and copyable
<!-- .element: class="fragment" -->

* Constrain the user to exactly **one** signature
<!-- .element: class="fragment" -->


Note:
- Has to fulfill **all** those requirements.

---

### Source

https://github.com/Voultapher/Presentations

Note:
- You'll find the source for both the presentation and examples here.

---

### Contact

* Email: lukas.bergdoll@gmail.com

* GitHub: https://github.com/Voultapher

Note:
- If you are watching this video, and you want to tell me how wrong I was.
- Here you go.

---

## Thanks

Note:
- Thanks for listening.

---

# Q&A

Note:
- Any remaining questions?
- [inplace vs `stdext::inplace_function`](http://quick-bench.com/Ls_1HAmdg1TyXRU-GE-BMKy17l4)
