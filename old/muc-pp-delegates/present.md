# The Situation
Note:
- The Situation >>
---
```cpp
void sort(range, comp_func)
```
templates allow for an elegant ZOA
<!-- .element: class="fragment" -->
```cpp
template<typename Func> void sort(range, Func comp_func)
```
<!-- .element: class="fragment" -->
Note:
- You want to give the user the option of customizing what your program does.
- Example std::sort **5s** >>
- templates allow for an elegant Zero overhead abstraction >> **5s** >>
---
But what if want to implement something like a task queue?
```cpp
template<typename Func> std::vector<Func> queue;
```
<!-- .element: class="fragment" -->
Note:
- But what if want to implement something like a task queue? >> 5s
- This works, however the user can only provide one kind of function type. >>
---
`std::tuple`
<!-- .element: class="fragment" -->
Note:
- More than one type you say? >>
- Sure, if you know at compile time how many function objects you'll have,
then `tuple` can be a **ZOA** >>
---
Enter `std::function`, the *magic* solution.
<!-- .element: class="fragment" -->
Note:
- Enter `std::function`, the >> *magic* solution. >>
---
So how do you implement *magic*?
Note:
- So how do you implement *magic*? >>
---
![alt text](dead_unicorn.jpg)
Note:
- Aparantly it involves the blood of a dead unicorn
and the tortured soul of a library vendor >>
---
![alt text](burst_bubble.jpg)
Note:
- Sorry to inform you :( There is no *magic*.
- As you'll find mentioned in every stackoverflow answer about this topic.
The price for *magic* is runtime. std::function is not a **ZOA** >>
---
## Maybe it's a bad pattern?
Note:
- Maybe you should avoid situations,
where you don't know the type at compile time >>
---
# **C++**
### you only pay for what you use
<!-- .element: class="fragment" -->
Note:
- One of C++'s core principles. >>
- You only pay for what you use >>
---
# closure
<!-- .element: class="fragment" -->
Note:
- Ok, before we dive deeper into the topic,
lets talk about some concepts.
- Oh yea, if you have questions, just raise your arm. >>
- Closure.
- A callable thing, that may contain state.
- Most common example. >>
---
```cpp
int val = 3;
auto lam = [val](int arg) -> int { return arg + val; };
```
Note:
- Lambdas! **10s** >>
---
```cpp
struct closure
{
    int val;
    explicit closure(int v) : val(v) {}

    int operator() (int arg)
    {
        return arg + val;
    }
};

int val = 3;
closure old{ val };
```
<!-- .element: class="fragment" -->
Note:
- This is not a new concept.
- Back before C++11, people had ways of expressing the same. >> **20s**
- It's just less pretty and more error prone >>
---
```cpp
for (int i = 1; i <= 10; ++i)
    std::cout << i << '\n';
```
<!-- .element: class="fragment" -->
Note:
- During the talk, we'll play a little game, where I show a small code snippet
and if it contains a `cout`, I want you to tell me what you think it prints.
- Let's give it a quick try >> **10s**
- Fantastic, let's go on. >>
---
## Size of callable *things*
Note:
- Size of callable *things* >>
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
- Let's start with function pointers **10s** >>
- On most platforms it's the size of a pointer, so 8 or 4 byte respectively >>
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
- Let's continue with lambdas **5s** >>
- 1 byte, so we already see a difference **5s** >>
- This time it's sizeof(int), usually 4 byte **5s** >>
- Again, 1 byte.
- The size of a closure depends soly on the size of what it captures >>
---
## Function pointer types
Note:
- Let's clear up some of the, at least for me more arcane syntax,
around function pointers >>
---
```cpp
void foo(int arg);
auto f = foo;
```
```cpp
decltype(f) == void(*)(int);
```
<!-- .element: class="fragment" -->
Note:
- What do you think the type of f is? **5s** It's: >>
- Still fairly reasonable >>
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
- How about member functions >> **5s**
- You guessed it, this is invalid, bar is private >>
---
```cpp
class foo
{
public:
    void bar(int arg);
};

auto f = &foo::bar;
```
```cpp
decltype(f) == void(foo::* bar)(int)
```
<!-- .element: class="fragment" -->
```cpp
f(3);
```
<!-- .element: class="fragment" -->
```cpp
foo obj;
(obj.*f)(3);
```
<!-- .element: class="fragment" -->
Note:
- So what's the type of f? **5s** >>
- Now how about calling f? >>
- Let me tell you, it's not **5s** >>
- It makes sense, that calling non static member functions,
need some way of accessing the current state of the object >>
---
```cpp
auto f = [](int arg) {};
```
```cpp
decltype(f) == decltype(f)
```
<!-- .element: class="fragment" -->
Note:
- How about this? **5s** >>
- I admit, that was a trick question.
- Every lambda has a unique type >>
---
## Alternatives
Note:
- Maybe we don't need magic. >>
---
## Coroutines
Note:
- For those unfamiliar with it, coroutines are a current **TS**
- With them you can suspend and resume functions >>
---
### The Basics
```cpp
void foo() {}
```
subroutine
<!-- .element: class="fragment" -->
```cpp
coro_return_type<int> test()
{
	co_await coro_awaitable_type{};
}
```
coroutine
<!-- .element: class="fragment" -->
Note:
- Just a quick introduciton >>
- This is a subroutine >> **3s**
- That is a coroutine >> **10s**
- co_await is a new keyword, coro_return_type is a coroutine_handle >>
---
```cpp
util::coro_task task_;

template<typename F> explicit delegate(F&& func)
{
    task_ = [](auto func) -> util::coro_task
    	{
    		co_await std::experimental::suspend_always{};
    		func();
    	}(std::forward<F>(func))
    );
}

```
<!-- .element: class="fragment" -->
Note:
- Diving right into it, a very simplified version >> **15s**
- The idea is that we store a coroutine_handle which points to
a suspended function, which once resumed will call our function. >>
---
## That's it?
Note:
- Before we try to fix all the problems with,
I suggest we benchmark it, and see if the idea works at all.
D: Switch to visual studio, and run benchmark

- That didn't work out :|. Why? Coroutines internally use type erasure,
again we have to store the closure somewhere.
- However this time, we are storing both the closure and the function state >>
---
## Function pointers
<!-- .element: class="fragment" -->
```cpp
int foo() { return 3; }

void bar()
{
    int(*f)() = foo;
    std::cout << (**********/*INLINE COMMENTS YAY*/***********f)();
}
```
<!-- .element: class="fragment" -->
Note:
- Not all hope is lost, we still have arguably the best C++ feature left >>
- Yes, function pointers >> **10s**
- Yes that's valid C++. What does it print?
- If you are confused by what you are looking at,
watch James McNeills great lightning talk from this years MeetingC++
---
```cpp
void(*f)(int) = [](int arg) { std::cout << arg; };
f(3);
```
<!-- .element: class="fragment" -->
Note:
- On a more serious note, we'll see that they can be very useful >> **5s**
- Lambdas are convertible to function pointers >>
---
```
class delegate
{
public:
    using invoke_ptr_t = void(*)(int);

    explicit delegate(invoke_ptr_t f) : invoke_ptr_(f) {}

    void operator() (int arg)
    {
        invoke_ptr_(arg);
    }
private:
    invoke_ptr_t invoke_ptr_;
};
```
Note:
- A very basic delegate implementation using function pointers **20s** >>
---
```cpp
delegate del{ [](int arg) { std::cout << arg; } };
del(3);
```
Note:
- It's your time again **5s** >>
---
# Generic
Note:
- [With the help of templates we can make it generic](https://godbolt.org/g/jA9tQn)
- Words >>
---
```cpp
int arr[10];
arr[0] = 2;

delegate<void(int)> del{
    [arr](int arg)
    { std::cout << arg + arr[0]; }
};

del(1);
```
<!-- .element: class="fragment" -->
Only captureless lambdas are convertible to function pointers
<!-- .element: class="fragment" -->
Note:
- We are done right? Although there is one small problem. Anyone an idea? >>
- **15s** That's invalid! >>
- There is a good reason for that, think back to the size of closures.
- sizeof(delegate) is the size of one function pointer,
where should it store the closure?
- The array is larger than the entire delegate class,
there is literally no way of stuffing the information in there >>
---
## Convert to functional programming
<!-- .element: class="fragment" -->
Note:
- The solution is simple >>
- Only use pure functions >>
---
# static
<!-- .element: class="fragment" -->
Note:
- Maybe, there is a way of constructing a function pointer
with a capturing lambda.
- The issue is, that we have to store the closure somewhere
- Enter C++'s least ambiguous keyword >> >>
---
```cpp
static int val = 4;
auto pure = [](int arg)
{
    return arg + val;
};
std::cout << pure(-1);
```
```cpp
int(*f)(int) = pure;
```
<!-- .element: class="fragment" -->
Note:
- **10s** Pure lambdas aren't that pure after all, this works because
the compiler can resolve the address of val at compile time
- Now we can do >> **3s** >>
---
Note:
- [With that we can add a new constructor to our
delegate class](https://godbolt.org/g/fxhIh7)
---
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
using del_t = delegate<int(void)>;

std::vector<del_t> vec;

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
Easy to use correctly
<!-- .element: class="fragment" -->

Easy to use incorrectly
<!-- .element: class="fragment" -->
Note:
- Sure we could tell the user to never do that.
- However this would result in an interface that is
- Easy to use correctly >> and >> Easy to use incorrectly
- When truly what we want is >>
---
Easy to use correctly

Hard to use incorrectly
<!-- .element: class="fragment" -->
Note:
- Easy to use correctly >> and >> Hard to use incorrectly
---
### How about we store the closure inplace
Note:
- We could just store the closure inside the delegate object. >>
---
```
template<typename R, typename... Args> class delegate<R(Args...)>
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
- It is a byte array, with limited size and alignment >>
---
```cpp
{
	std::aligned_storage<sizeof(int), alignof(int)> storage;
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
```cpp
{
	std::aligned_storage<sizeof(int), alignof(int)> storage;
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
```cpp
{
	std::aligned_storage<sizeof(int), alignof(int)>::type storage;
	new(&storage)int{ 3 };

	std::cout << reinterpret_cast<int&>(storage);
}
std::cout << 6;
```
Note:
- We forgot a ::type at the end of aligned_storage. **2s**
- Trust me you don't want to debug, that.
- By writing into storage, which essentially was an empty
struct, we corrupted the stack >>
---
Note:
- [With that we can change delegate to](https://godbolt.org/g/NnRBAs)
- Althoght that is currentlyUB
- So, now that we successfully stored the closure,
how do we get it back out?
- Again we have no way of making the type visible to the rest of the class.
- operator() invokes the function pointer.
- How about we change the function pointer type? >>
---
```cpp
template<typename T> explicit delegate(T&& closure)
{
	new(&storage_)T{ std::forward<T>(closure) };

	invoke_ptr_ = static_cast<invoke_ptr_t>(
		[](storage_t& storage, Args&&... args) -> R
    	{
    		return reinterpret_cast<T&>(storage)(
                std::forward<Args>(args)...
            );
    	}
    );
}
```
Note:
- I'll give a minute to look at it. **45s**
- This works because inside a lambda all current types are visible
- Note the change to the function signature.
- That solves our forwarding issue.
---
Note:
- [So our entire class now looks like this](https://godbolt.org/g/Toju8d)
- Words >>
---
# ~~ZOA~~
Note:
- However, this is no longer a **ZOA**
- If we construct it with a pure function, we'll pay none the less >>
---
How about we split interface and implementation
* pure
<!-- .element: class="fragment" -->
* inplace_triv
<!-- .element: class="fragment" -->
* inplace
<!-- .element: class="fragment" -->
* dynamic
<!-- .element: class="fragment" -->
Note:
- Let's create one implementation per use case. >>
- pure, only works with pure functions. >>
- inplace_triv stores the closure inplace,
only works with trivially constructable and destructable types. >>
- inplace stores the closure inplace. >>
- dynamic stores the closure on the heap.
- More on why we should make that separation, later. >>
---
```cpp
~inplace()
{
	// call closure destructor
}
```
<!-- .element: class="fragment" -->
Note:
- Both pure and inplace_triv have default copy, move and destruction.
- With inplace it gets a bit more tricky, we cannot just copy the storage
object byte for byte.
- Again, same issue, how do we get the closure out of
storage in those operations?
- Let's first look at the destruction. >> **3s** >>
---
# MORE FUNCTION POINTERS
<!-- .element: class="fragment" -->
Note:
- The answer is obviously >>
- Sadly, I'm not joking >>
---
```cpp
template<typename T> explicit inplace(T&& closure)
{
    // ... same as before

    destructor_ptr_ = static_cast<destructor_ptr_t>(
		[](storage_t& storage) noexcept -> void
		{ reinterpret_cast<T&>(storage).~T(); }
	);
}
```
Note:
- We'll store a second function pointer **10s** >>
---
```cpp
~inplace()
{
	destructor_ptr_(storage_);
}
```
<!-- .element: class="fragment" -->
Note:
- Now we can do. >> **5s** >>
---
```cpp
template<typename T> explicit inplace(T&& closure)
{
    // ... same as before

    copy_ptr_ = copy_op<T, storage_t>();
}
```
<!-- .element: class="fragment" -->
Note:
- Similar story for the copy operations >> **5s**
- So why put another layer of indirection in there? >>
---
```cpp
template<
    typename T,
    typename S,
    typename std::enable_if_t<
    std::is_copy_constructible<T>::value, int
    > = 0
> copy_ptr_t copy_op()
{
    return [](S& dst, S& src) noexcept -> void
    {
        new(&dst)T{ reinterpret_cast<T&>(src) };
    };
}
```
<!-- .element: class="fragment" -->
Note:
- Let's look at the implementation of copy_op. >> **15s**
- It's fairly straight forward, we write into the local storage object,
the closure that is currently stored in the source object.
- But why the enable_if? Let's look a the other overload >>
---
```cpp
template<
    typename T,
    typename S,
    typename std::enable_if_t<
    !std::is_copy_constructible<T>::value, int
    > = 0
> copy_ptr_t copy_op()
{
    static_assert(std::is_copy_constructible<T>::value,
        "constructing delegate with move only type is invalid!");
}
```
Note:
- **10s**
- As you can see, it is just a dummy function that'll fail on instantiation.
- Why can't we build a delegate with move only types? >>
---
```cpp
del_t del_a = rand_bool ? copy_move_closure{} : move_only_closure{};

del_t del_b = del_a; // can we copy?
```
<!-- .element: class="fragment" -->
Note:
- Consider this example >> **10s**
- We would have to make decisions at compile time about run time properties.
- I see 2 possible solutions,
 if you *really* want to build delegates with move only types.
- You could either make a move only delegate, or have a throwing copy. >>
---
```cpp
inplace(const inplace& other) :
    invoke_ptr_{ other.invoke_ptr_ },
    copy_ptr_{ other.copy_ptr_ },
    destructor_ptr_{ other.destructor_ptr_ }
{
    copy_ptr_(storage_, other.storage_);
}
```
<!-- .element: class="fragment" -->
Note:
- The copy constructor is fairly straight forward >> **10s**
- Note, we have to use the new copy pointer >>
---
```
inplace& operator= (const inplace& other)
{
    if (this != std::addressof(other))
    {
        invoke_ptr_ = other.invoke_ptr_;
        copy_ptr_ = other.copy_ptr_;

        destructor_ptr_(storage_);
        copy_ptr_(storage_, other.storage_);
        destructor_ptr_ = other.destructor_ptr_;
    }
    return *this;
}
```
<!-- .element: class="fragment" -->
Note:
- The copy assignment is similar >> **15s**
- This time we have to destroy our closure before copying >>
---
```cpp
R operator() (Args&&... args) const
{
    return invoke_ptr_(storage_, std::forward<Args>(args)...);
}
```
<!-- .element: class="fragment" -->
Note:
- In order to stay close to std::function's api,
the calling operator should be marked const. >> **5s** >>
---
```cpp
private:
	mutable storage_t storage_;

	invoke_ptr_t invoke_ptr_;
	copy_ptr_t copy_ptr_;
	destructor_ptr_t destructor_ptr_;
```
<!-- .element: class="fragment" -->
Note:
- Still, we could be modifying storage,
so we have to mark storage as mutable. >> **5s** >>
---
## Designing the interface
Note:
- Designing the interface >>
---
Why did we even split interface and implementation?

How do we bring it all together?
<!-- .element: class="fragment" -->
Note:
- Initially I didn't.
- There were multiple constructors and flags.
- After a while is was complicated to keep track of all the possibilities,
due to all the conditionality involved,
depending on what it was constructed with. >>
- How do we bring it all together? >>
---
# Variant
Note:
- A new C++17 feature, based on an old concept.
- It is basically an *or* rather than an *and* struct.
- I'll not go into to much detail here, if you want to know more,
there is a good talk from this years cppcon called:
"Variants: Past, Present, and Future" by David Sankel. >>
---
Note:
- Let's see it in action
- [std::function version](https://godbolt.org/g/uUVZvz)
- [variant version](https://gist.github.com/Voultapher/42964ed34db8e0b26d88f6f00a0db010)
- maybe it's a bad implementation, let's try another
- [when truely all we wanted is](https://godbolt.org/g/qHCZmG)
- Not only is it not a **ZOA** anymore,
- It's not even better than plain std::function. >>
---
Was all that for nothing?

Is there just no better way of solving this problem?
<!-- .element: class="fragment" -->
Note:
- At that point I was somewhat hitting a wall.
- I was frustrated, it all looked so good,
but again and again bugs forced me to abandon my previous ideas. >>
After a couple of days, filled with the fruitless endeavor of,
saving a sunken ship, I finally realized:
- Yes, there is no better way of solving this problem >>
---
## I was trying to solve the wrong problem
Note:
- I was trying to solve the wrong problem >>
---
# **C++**
### you only pay for what you use
<!-- .element: class="fragment" -->
Note:
- What did we say earlier about C++? Right. >>
- you only pay for what you use.
- But we have no idea what the user will use.
- Let's make the user tell us what he want s.
- You tell us what you want, and that's what you'll get.
- No background magic trying to figure out what the user wants. >>
---
```cpp
template<
	typename T,
	template<size_t, typename, typename...>class Spec = spec::inplace,
	size_t size = detail::default_capacity
>
class delegate; // unspecified

template<
	typename R, typename... Args,
	template<size_t, typename, typename...>class Spec,
	size_t size
>
class delegate<R(Args...), Spec, size>;
```
<!-- .element: class="fragment" -->
Note:
- Here is what it looks like >> **20s**
- We let the user decide what kind of limitations they want.
- And we even provide a default. >>
---
[Let's take a look a the result](https://gist.github.com/Voultapher/2ee61395ea86c49d886465ceb8b0b214)
Note:
- Let's take a look a the result:
- D: show result >>
---
```cpp
R operator() (Args&&... args) const
{
    if (empty)
        throw std::bad_function_call();

    return invoke_ptr_(storage_, std::forward<Args>(args)...);
}
```
<!-- .element: class="fragment" -->
Note:
- I'd like a world where people are ok with no empty constructor,
it results in cleaner code.
- However it seems people want empty construction,
and if called empty, it should explode nicely.
- We could just add a boolean flag, empty. >> **7s**
- That's not nice. Maybe there is a better way. >>
---
```cpp
template<
	typename R,
	typename S,
	typename... Args
> static R empty_inplace(S&, Args&&... args)
{
	throw std::bad_function_call();
}
```
<!-- .element: class="fragment" -->
Note:
- What exactly are we building here?
- We are storing function objects, so why not store a throwing function?
- Said function: >> **5s**
- And the empty constructor >>
---
```cpp
explicit inplace() noexcept :
    invoke_ptr_{ empty_inplace<R, storage_t, Args...> },
    copy_ptr_{ copy_op<std::nullptr_t, storage_t>() },
    destructor_ptr_{ [](storage_t&) noexcept -> void {} }
{
    new(&storage_)std::nullptr_t{ nullptr };
}
```
Note:
- **15s**
- That's all we have to change.
- Making checking for emptyness equally simple. >>
---
```cpp
bool empty() const noexcept
{
	return reinterpret_cast<std::nullptr_t&>(storage_) == nullptr;
}
```
Note:
- **5s**
- That's it. Sure we glossed over a lot of details.
- None the less, those are all the important pieces you need. >>
---
## Benchmarks
Note:
- Let's do some Benchmarks.
---
## Test driven development
<!-- .element: class="fragment" -->
Note:
- I want to take some time,
and talk about applied design philosophies. >>
- Test driven development
- Whenever I develop software, I think about how you, the user could break it.
- So as I kept finding new bugs,
whilst at the same time rewriting large chunks of the code,
it helped tremendously to offload a thought like,
"could it fail if that happens?", to a test case. >>
---
## Type oriented design
<!-- .element: class="fragment" -->
Note:
- Did you notice, we barely used any control flow statements?
- In this entire 600 line, implementation there are only 2 ifs.
- The ones in the inplace copy and move assignments.
- That's it. No switch, no loops.
- Why is that? >>
- Fundamentally, every program is some sort of data transformations.
- Take data, transform it and put it somewhere.
- Ask yourself, what is a type?
- At its core, a type describes a memory layout,
so what is more ideal than a type, for describing data transformations? >>
---
## Lessons learned
* Do not be afraid to challenge a status quo!
<!-- .element: class="fragment" -->
* The price of magic is runtime
<!-- .element: class="fragment" -->
* Be responsible for your state
<!-- .element: class="fragment" -->
Note:
- Let us look at what we learned >>
- Do not be afraid to challenge a status quo!
- Sometimes it takes redefining the problem to progress. >>
- The price of *magic* is runtime
- Interfaces that try to guess what the user want's are fundamentally doomed,
such as the bloated mess that is OpenGL. >>
- Be responsible for your state
- When it comes to state, it's like having children, it's your's.
- Don't just give it to someone and hope they'll take good care of it. >>
---
# Questions
Note:
- I'll start >>
---
How do you know it works?
Note:
- I don't. However, [this may convince you](139.59.214.141:8080) >>
---
## Links:
* email: lukas.bergdoll@gmail.com
* [github]()
* [James McNellis - "my favorite C++ feature"](https://youtu.be/6eX9gPithBo)
* [David Sankel - "Variants: Past, Present, and Future"](https://youtu.be/k3O4EKX4z1c)
* [Full implementation](https://github.com/hioa-cs/IncludeOS/blob/dev/api/util/delegate.hpp)
---
# Hiring?
Note:
- Shameless plug, if you are hiring,
I'm currently looking.
