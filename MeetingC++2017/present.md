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
- Is `std::function` the best it can do?
- Yes!
- Is `std::function` the best you can do? >>

---

# No

Note:
- No!


---

### Roadmap

* TODO
<!-- .element: class="fragment" -->
* Client
<!-- .element: class="fragment" -->
* Server
<!-- .element: class="fragment" -->

Note:
- First we'll try to see where a C++ application makes sense. >>
- Then a quick a look at the client side of a web application. >>

---

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
- Change auto to int

---

Note:
- No one would write code like this, right?
- Github search
- Majority was free functions

---

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

Note:
- TODO try to replace with templates

---

```cpp
template<typename T>
std::vector<T> filter(
  std::function<bool(T)> pred,
  const std::vector<T> & vec
);
```

Note:
- Tried being generic

---

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

---

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

---


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
std::function<void()> a = []() { return 42; };
```
<!-- .element: class="fragment" -->
```cpp
std::array<int, 4> buf;
std::function<void()> a = [buf]() { return 42; };
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
- Here?
- We really can't know here. Depends on the caller. >>

---

# closure

Note:
- Ok, before we dive deeper into the topic,
lets talk about some concepts.
- Closure.
- A callable thing, that may contain state.
- Most common example. >>

---

```cpp
int val = 3;
auto lam = [val](int arg) -> int { return arg + val; };
```

Note:
- Lambdas
- Not new concept

---

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

Note:
- During the talk, when I show you a small code snippet containing `cout`,
I want you to tell me what you think it prints.
- Let's give it a quick try.

---

## Size of callable *things*
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

8 or 4 byte
<!-- .element: class="fragment" -->

Note:
- Let's start with function pointers
- On most platforms it's the size of a pointer, so 8 or 4 byte respectively

---

Lambdas

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

## Function pointer types

Note:
- Function pointers
- Arcane syntax

---

### Free Functions

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

### Member function pointers

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

### Member function pointers

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

### Lambda ->
### Function Pointer

```
void(*f)(int) = [](int arg) { std::cout << arg; };

f(3);
```

Note:
- Lambdas are convertible to function pointers.

---

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
- Basic function closure implementation.
- Ignoring rule of 6 to make it more readable.

---

```cpp
function f{[](int a) { std::cout << a + 2; }};
f(5);
```

Note:
- Now possible.

---

### Generic

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

  R operator() (Args&&... args) const
  {
    return invoke_ptr_(std::forward<Args>(args)...);
  }
private:
  invoke_ptr_t invoke_ptr_;
};
```

Note:
- Make generic
- Class signature
- `invoke_ptr_t`
- Call operator

---

### Usage

```cpp
function<int(int)> f{
  [](int a) { return a + 3; }
};

const int res = f(5);
```

Note:
- Example usage
- What are we missing?

---

### Closure Storage

```cpp
const int b;
function<int(int)> f{
  [b](int a) { return a + b; }
};

const int res = f(5);
```

---

### Compiler Error

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

## Convert to functional programming7
<!-- .element: class="fragment" -->

Note:
- The solution is simple
- Only use pure functions

---

# static
<!-- .element: class="fragment" -->

Note:
- Maybe, there is a way of constructing a function pointer
with a capturing lambda.
- The issue is, that we have to store the closure somewhere.
- Enter C++'s least ambiguous keyword.

---

### Implicit Lambda Capture

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

```
const int val = 4;
auto pure = [](int arg)
{
  return arg + val;
};
std::cout << pure(-1);
```
<!-- .element: class="fragment" -->

Note:
- Pure lambdas not really pure
- Compiler can resolve the address of val at compile time.
- Now we can do >> **3s** >>
- Something curious I found out just recently.
- Nope, odr usage. Standard 7.2.
- With that we can add a new constructor to our function class.

---

### Static Closure Storage

```cpp
thread_local static T cap{ std::forward<T>(closure) };

invoke_ptr_ = static_cast<invoke_ptr_t>([](Args... args) -> R
{
  return cap(std::forward<Args>(args)...);
});
```

Note:
- We are golden now, right?
- Someone see any issues? **10s**
- We haven't talked about copying and moving, but let's do that later.
- We are doing improper argument type forwarding.
- To avoid unnecessary moving, it should be >>

---

```cpp
thread_local static T cap{ std::forward<T>(closure) };

invoke_ptr_ = static_cast<invoke_ptr_t>([](Args&&... args) -> R
{
  return cap(std::forward<Args>(args)...);
});
```
Note:
- We should be passing in the arguments as rvalue references. **5s**
- But that does not work, as that would not fit the function pointer type. >>

---

```cpp
using func_t = function<int(void)>;

std::vector<func_t> vec;

for (int i = 0; i <= 3; ++i)
  vec.emplace_back([i]() { return i; });

std::cout << vec.back()();
```
<!-- .element: class="fragment" -->

```cpp
thread_local static T cap{ std::forward<T>(closure) };
```
<!-- .element: class="fragment" -->
Note:
- Let's look at this >> **15s**
- You guessed it, it pirnts 0.
- Although each lambda has a unique type, here there is only **ONE**
lambda, which means that, >>
- cap gets created only once. **3s**
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

### How about we store the closure inplace

Note:
- We could just store the closure inside the function object. >>

---

```py
template<typename R, typename... Args>
class function<R(Args...)>
{
public:
  using invoke_ptr_t = R(*)(Args...);
  using storage_t = ???
}
```
<!-- .element: class="fragment" -->

Note:
- Now what should the local variable type be? >> **5s**
- The type of the closure does only become visible in the constructor,
as far as I know there is no way of making that type visible to other parts
of the class.
- It seems we may have to resort to the dark art of type less storage >>
---

`void*`
<!-- .element: class="fragment" -->

Note:
- The type less storage type most of you know is >>
- But we want to avoid heap allocation.
- As luck would have it, even here the deep pockets of C++11 got us covered >>

---

`std::aligned_storage`

Note:
- It is a byte array, with specific size and alignment >>

---

### Inplace Stack Storage

```cpp
{
  std::aligned_storage<
    sizeof(int),
    alignof(int)
  > storage;

  new(&storage)int{ 3 };

  std::cout << reinterpret_cast<int&>(storage);
}
std::cout << 6;
```

Note:
- So what does this print? **15s** >>

---

# UB

Note:
- Hmm.. >>

---

### Inplace Stack Storage

```cpp
{
  std::aligned_storage<
    sizeof(int),
    alignof(int)
  > storage;

  new(&storage)int{ 3 };

  std::cout << reinterpret_cast<int&>(storage);
}
std::cout << 6;
```

Note:
- Yes it prints 3, and then it corrupts the stack trying to leave
the inner scope. **5s**
- Who can spot the bug? >>

---

### Inplace Stack Storage

```cpp
{
  std::aligned_storage<
    sizeof(int),
    alignof(int)
  >::type storage;

  new(&storage)int{ 3 };

  std::cout << reinterpret_cast<int&>(storage);
}
std::cout << 6;
```


Note:
- We forgot a ::type at the end of aligned_storage. **2s**
- Trust me you don't want to debug, that.
- By writing into storage, which essentially was an empty struct,
we corrupted the stack
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
we have to understand the conceptual memory time line.
- Class level: here we decide the memory layout.
- Constructor level: here we decide how the memory layout is initialized.
Note this still happens at compile time, however the constructor can
no longer change the memory layout.
- The memory layout is strongest way for the compiler to reason about
your program, with that comes the biggest optimization potential.
- Run time: here we potentially mutate the memory.

---

When should you use [`std::function`]()?

* Has to be stored inside a container
<!-- .element: class="fragment" -->
* You really can't know the layout at compile time
<!-- .element: class="fragment" -->
* You really can't know the layout at compile time
<!-- .element: class="fragment" -->
* All your closure types are move and copyable
<!-- .element: class="fragment" -->
* Constrain the user to exactly one signature
<!-- .element: class="fragment" -->


Note:
- Has to fulfill **all** those requirements.

---

sg14 inplace_function
How big to choose inline size.
Store vtable pointer ala `dyno::function`
[Price of loosing inlining](http://quick-bench.com/cDenhM9zX6NFQbdUXJQV2FfmtuY)
Note, here the vector reserve is an allocation, so that's part of the cost.
The `std::function` object should be using sbo, only storing one int.
Someone else comes along, and adds vec to the capture list.
Note, PGO can help optimize deferred calls.

---

If it's a free function, using templates is basically always a better option.




I won't show you any benchmarks in this talk. Benchmarks, especially about
this topic can be misleading.
Implementation can limit inlining.
Implementation can cause heap allocation.
Alternate implementations, like inline storage can make iteration worse.
Perfect switch case reinterpret_cast trash instruction cash.

Note how I always say can, not will. Using run time polymorphism,
will make your performance much less predictable.

Point to Louis' talk to learn more about alternate implementations.

Realize what you are opting into!

Coroutine move only function

Personally, I try to avoid `std::function` whenever possible.
I think it is a bad abstraction most of the time.
It only make sense in very specific library level use cases.

Function signature issue, usually over constraining.
