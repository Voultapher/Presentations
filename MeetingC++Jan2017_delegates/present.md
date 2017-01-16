# The Situation
Note:
- The Situation >>
---
```cpp
void sort(range, comp_func)
```
templates allow for an elegant ZOA solution
<!-- .element: class="fragment" -->
```cpp
template<typename Func> void sort(range, Func comp_func)
```
<!-- .element: class="fragment" -->
Note:
- You want to give the user the option of customizing what your program does.
- Example std::sort >>
- ZOA Zero overhead abstraction >> >>
---
But what if want to implement something like a task queue?
```cpp
template<typename Func> std::vector<Func> queue;
```
<!-- .element: class="fragment" -->
Note:
- But what if want to implement something like a task queue? >>
- This works, however the user can only provide one kind of function type. >>
---
`std::tuple`
<!-- .element: class="fragment" -->
Note:
- More than one type you say? >>
- If you know at compile time how many function objects you'll have,
then `std::tuple` can be a **ZOA** >>
---
Enter `std::function`, the *magic* solution.
<!-- .element: class="fragment" -->
Note:
- Enter `std::function`, the -> *magic* solution. >>
---
So how do you implement *magic*?

Image of dead unicorn
<!-- .element: class="fragment" -->
Note:
- So how do you implement *magic*? >>
- Aparantly it involves the blood of a dead unicorn
and the tortured soul of a library vendor >>
---
Image of burst bubble.
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
- A callable thing, that may contain state.
- Most common example. >>
---
```cpp
int val = 3;
auto lam = [val](int arg) -> int { return arg + val; };
```
Note:
- Lambdas! >>
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
- Back before C++11, people had ways of expressing the same. >>
- It's just less pretty and more error prone >>
---
```cpp
for (int i = 1; i <= 10; ++i)
    std::cout << i << '\n';
```
<!-- .element: class="fragment" -->
Note:
- During the talk, we'll play a game, where I show a small code snippet
and if it contains a cout, you tell me what it prints
- Let's give it a quick try >>
- I'm amazed to have such a qulified audience >>
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
- Let's start with function pointers >>
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
- Let's continue with lambdas >>
- 1 byte, so we already see a diference >>
- This time it's sizeof(int), usually 4 byte >>
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
- You guessed it, it's: >>
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
- How about member functions >>
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
- So what's the type of f? >>
- Now how about calling f? >>
- Nope >>
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
- So, how about this? >>
- You guessed right, this was a trick question.
- Every lambda has a unique type >>
---
# Alternatives
Note:
- Maybe we don't need magic. >>
---
# Coroutines
Note:
- For those unfamiliar with it, coroutines are a current **TS**
- With them you can suspend and resume functions
---
### The Basics
```cpp
void foo() {}
```
```cpp
coro_return_type<int> test()
{
	co_await coro_awaitable_type{};
}
```
<!-- .element: class="fragment" -->
Note:
- This is a subroutine >>
- That is a coroutine >>
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
Note:
- A simplified version with just the important parts,
- The idea is that we store a coroutine_handle which points to
a suspended function, which once resumed will call our function. >>
---
## That's it?
Note:
- Let's benchmark it
D: Switch to visual studio, and run benchmark

- That didn't work out :|. Why? Coroutines internally use type erasure,
again we have to store the state somewhere,
however this time we are storing both the closure and the function state
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
- Not all hope is lost, we still have arguably the best C++ feature left >> >>
- Yes that's valid C++. What does it print?
- If you are confused by what you are looking at,
- watch James McNeills great lightning talk from this years MeetingC++
---
```cpp
void(*f)(int) = [](int arg) { std::cout << arg; };
f(3);
```
<!-- .element: class="fragment" -->
Note:
- On a more serious note, we'll see that they can be very usefull >>
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
- A very basic delegate implementation using function pointers >>
---
```cpp
delegate del{ [](int arg) { std::cout << arg; } };
del(3);
```
Note:
- It's your time agian >>
---
Note:
- With the help of templates we can make it generic
D: show in godbolt
```
template<typename T> class delegate; // unspecified

template<typename R, typename... Args> class delegate<R(Args...)>
{
public:
	using invoke_ptr_t = R(*)(Args...);

	explicit delegate(invoke_ptr_t f) : invoke_ptr_(f) {}

	R operator() (Args... args)
	{
		return invoke_ptr_(std::forward<Args>(args)...);
	}
private:
	invoke_ptr_t invoke_ptr_;
};

void foo()
{
	delegate<void(int)> del{ [](int arg) { std::cout << arg; } };
	del(3);
}
```
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
- That's invalid >>
- There is a good reason for that, think back to the size of closures.
- sizeof(delegate) is the size of one function pointer,
where should it store the closure?
- The array is larger than the entire delegate class,
there is literraly no way of stuffing the information in there >>
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
- Enter C++'s least ambigious keyword >> >>
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
- Pure lambdas aren't that pure after all, this works because
the compiler can resolve the address of val at compile time
- Now we can do >>
---
Note:
- With that we can add a new constructor to delegate
D: show on godbolt
```
template<typename T> class delegate; // unspecified

template<typename R, typename... Args> class delegate<R(Args...)>
{
public:
	using invoke_ptr_t = R(*)(Args...);

	explicit delegate(invoke_ptr_t f) : invoke_ptr_(f) {}

	template<typename T> explicit delegate(T&& closure)
	{
		thread_local static T cap{ std::forward<T>(closure) };

		invoke_ptr_ = static_cast<invoke_ptr_t>([](Args... args) -> R
		{
			return cap(std::forward<Args>(args)...);
		});
	}

	R operator() (Args... args)
	{
		return invoke_ptr_(std::forward<Args>(args)...);
	}
private:
	invoke_ptr_t invoke_ptr_;
};

void test()
{
	int val = 3;
	delegate<void()> del{ [val]() { std::cout << val; } };
	del();
}
```
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
- Someone see any issues?
- We haven't talked about copying and moving, but let's do that later.
- We are doing inproper argument type forwarding.
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
- We should be passing in the arguments as rvalue references.
- But that does not work, as that would not fit the function pointer type. >>
---
```cpp
using del_t = delegate<int(void)>;

std::vector<del_t> vec;

for (int i = 0; i <= 3; ++i)
    vec.emplace_back([i]() { return i; });

std::cout << vec.front()();
```
<!-- .element: class="fragment" -->
```cpp
thread_local static T cap{ std::forward<T>(closure) };
```
<!-- .element: class="fragment" -->
Note:
- Let's look at this >>
- You guessed it, again it prints 3
- Although each lambda has a unique type, here there is only **ONE**
lambda, which means that, >>
- gets created only once, the first time this constructor is called.
---
Easy to use correctly
<!-- .element: class="fragment" -->

Easy to use incorrectly
<!-- .element: class="fragment" -->
Note:
- Sure we could tell the user to never do that.
- However this would result in an interface that is
- Easy to use correctly >> and >> Easy to use incorrectly
- When truely what we want is >>
---
Easy to use correctly

Hard to use incorrectly
<!-- .element: class="fragment" -->
Note:
- Easy to use correctly >> and >> Hard to use incorrectly
---
## How about we store the closure inplace
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
- Now what should the local varible type be? >>
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
- So what does this print?
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
the inner scope.
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
- We forgot a ::type at the end of aligned_storage.
- Trust me you don't want to debug, that.
- By writing into storage, which essentally was an empty
struct, we corrupted the stack >>
---
Note:
- With that we can change delegate to:
D: show on godbolt
```
template<typename T> class delegate; // unspecified

template<typename R, typename... Args> class delegate<R(Args...)>
{
public:
	using invoke_ptr_t = R(*)(Args...);

	using storage_t = std::aligned_storage<
		sizeof(size_t) * 4,
		sizeof(size_t)
	>::type;

	template<typename T> explicit delegate(T&& closure)
	{
		new(&storage_)T{ std::forward<T>(closure) };

        //invoke_ptr_ = ???
	}

	R operator() (Args... args)
	{
		return invoke_ptr_(std::forward<Args>(args)...);
	}
private:
	invoke_ptr_t invoke_ptr_;
	storage_t storage_;
};

void test()
{
	int val = 3;
	delegate<void()> del{ [val]() { std::cout << val; } };
	del();
}
```
- So, now we successfully stored the closure, but how do we get it back out?
- Again we have no way of making the type visible to the rest of the class,
- operator() invokes the function pointer.
- So how about we change the function pointer type? >>
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
	});
}
```
Note:
- I'll give a minute to look at it.
- This works because inside a lambda all current types are visible
- Note the change to the function signature.
- That solves our forwarding issue.
---
Note:
- So our entire class now looks like this
D: show on godbolt
```
template<typename T> class delegate; // unspecified

template<typename R, typename... Args> class delegate<R(Args...)>
{
public:
	using storage_t = std::aligned_storage<
		sizeof(size_t) * 4,
		sizeof(size_t)
	>::type;

	using invoke_ptr_t = R(*)(storage_t&, Args&&...);

	template<typename T> explicit delegate(T&& closure)
	{
		new(&storage_)T{ std::forward<T>(closure) };

		invoke_ptr_ = static_cast<invoke_ptr_t>(
			[](storage_t& storage, Args&&... args) -> R
		{
			return reinterpret_cast<T&>(storage)(std::forward<Args>(args)...);
		});
	}

	R operator() (Args... args)
	{
		return invoke_ptr_(storage_, std::forward<Args>(args)...);
	}
private:
	invoke_ptr_t invoke_ptr_;
	storage_t storage_;
};

void test()
{
	int val = 3;
	delegate<void()> del{ [val]() { std::cout << val; } };
	del();
}
- Words >>
```
---
# ~~ZOA~~
Note:
- However, this is no longer a **ZOA**
- If we construct it with a pure function, we'll pay none the less >>
---
How about we split interface and implementation
<!-- .element: class="fragment" -->
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
only works with trivailly constructable and destructable types. >>
- inplace stores the closure inplace.
- dynamic stores the closure on the heap.
- More on why we should make that seperation, later. >>
---
```cpp
~inplace()
{
	// call closure destructor
}
```
Note:
- Both pure and inplace_triv have default copy, move and destruction.
- With inplace it gets a bit more tricky, we cannot just copy the storage
object byte for byte.
- Agian, same issue, how do we get the closure out of
storage in those operations?
- Let's first look at the destruction. >>
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
- We'll store a second function pointer >>
---
```cpp
~inplace()
{
	destructor_ptr_(storage_);
}
```
<!-- .element: class="fragment" -->
Note:
- Now we can do. >>
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
- Similar story for the copy operations >>
- So why put another layer of indirection in there?
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
Note:
- Let's look at the implementation of copy_op. >>
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
- As you can see, it is just a dummy function tha'll fail on instanciation.
- Why can't we build a delegate with move only types? >>
---
```cpp
del_t del_a = rand_bool ? copy_move_closure{} : move_only_closure{};

del_t del_b = del_a; // can we copy?
```
<!-- .element: class="fragment" -->
Note:
- Consider this example >>
- We would have to make decisions at compile time about run time properties.
- I see 2 possible solutions if you *really* want move only delegates.
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
- The copy constructor is fairly straight forward >>
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
- The copy assignment is similar >>
- Notice, this time we have to destroy our closure before copying >>
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
the calling operator should be marked const. >> >>
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
so we have to mark storage as mutable. >> >>
---
# Designing the interface
---
Why did we even split interface and implementation?

How do we bring it all together?
<!-- .element: class="fragment" -->
Note:
- Initially I didn't.
- There were multiple constructors and flags.
- After a while is was coplicated to keep track of all the posibilites,
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
D: Show on godbolt std::function version,
D: then variant version,
D: then show just calling pure
- Not only is it not **ZOA** anymore,
- It's not even better than plain std::function. >>
---
Was all that for nothing?
<!-- .element: class="fragment" -->

Is there just no better way of solving this problem?
<!-- .element: class="fragment" -->
Note:
- At that point I was somewhat hitting a wall.
- I was frustrated, it all looked so good,
but again and again bugs forced me to abandon my previous ideas. >>
After a couple of days, filled with the fruitless endevor of,
saving a sunken ship, I finally realized:
- Yes there is no better way of solving this problem >>
---
# I was trying to solve the wrong problem
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
- Let's make the user tell us what he wants.
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
- Here is what it looks like >>
- We let the user decide what kind of limitations they want.
- And we even provide a default. >>
---
D: Show how it works on godbolt
Note:
- Words: >>
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
- However it seems people want empyt construction,
and if called empty it should explode nicely.
- We could just add a boolean flag, empty. >>
- That's not nice. Maybe there is better way. >>
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
- What excatly are we building here?
- We are storing function objects, so why not store a thrwoing function?
- Said function: >>
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
- That's all we have to change. >>
- Making checking for emptyness equally simple. >>
---
```cpp
bool empty() const noexcept
{
	return reinterpret_cast<std::nullptr_t&>(storage_) == nullptr;
}
```
Note:
- That's it. Sure we glossed over a lot of details.
- None the less, those are all the important pieces you need. >>
---
# Benchmarks
Benchmark with and without small size optimization

# Lessons learned
Do not be afraid to challange a status quo!
By seperating interface from implementation, you get more freedom and if one implementation
works and the other does not. You know its not an interface issue.

Test drived development

---
Note:Did you notice, we barely used any controll flow statements?
Note:In this entire 600 line, implementation there are only 2 ifs.
Note:The ones in the inplace copy and move assignments.
Note:That's it. No switch, no loops.
Note:Why is that?
# Type oriented design
Note:Fundamentally, every program is some sort of data transformations.
Note:Take data, transform it and put it somewhere.
Note:Ask yourself, what is a type?
Note:At its core, a type describes a memory layout,
Note:so what is more ideal than a type, for describing data transformations?
---

---
# Questions
Note:I'll start
---
How do you know it works?
Note:I don't. However, this may convince you.
D: Show IncludeOS server.

### Links:
[James McNellis - "my favorite C++ feature"](https://youtu.be/6eX9gPithBo)
[David Sankel - "Variants: Past, Present, and Future"](https://youtu.be/k3O4EKX4z1c)
[Full implementation](https://github.com/hioa-cs/IncludeOS/blob/dev/api/util/delegate.hpp)

---
# Hiring?
Note:Shameless plug, if you are currently hiring,
feel free to talk to me. // better wording
