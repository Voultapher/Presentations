# The Situation
---
C: You want to give the user the option of customizing what your program does
Example std::sort
```
void sort(range, comp_func)
```
templates allow for an elegant **ZOA** solution
C: **ZOA** Zero overhead abstraction
```
template<typename Func> void sort(range, Func comp_func)
```
---
But what if want to implement something like a task queue?
```
template<typename Func> std::vector<Func> queue;
```
C:This works, however the user can only provide one kind of function type.
---
C: More than one type you say?
`std::tuple`
C: If you know at compile time how many function objects you'll have,
then `std::tuple` can be a **ZOA**
---
Enter `std::function`, the *magic* solution.
---
So how do you implement *magic*?
Image of dead unicorn
C: Aparantly it involves the blood of a dead unicorn
and the tortured soul of a library vendor
---
Image of burst bubble.
C: Sorry to inform you :( There is no *magic*.
As you'll find mentioned in every stackoverflow answer about this topic.
The price for *magic* is runtime. No more **ZOA**
---
# Maybe it's a bad pattern?
C: Maybe you should avoid situations where you don't know the type at compile time
---
# **C++** you only pay for what you use
---
C: Ok, before we dive deeper into the topic,
C: lets talk about some concepts.
C: Oh yea, if you have questions, just raise your arm.
### closure
C: A callable thing, that may contain state
C: most common example
```
int val = 3;
auto lam = [val](int arg) -> int { return arg + val; };
```
C: Lambdas. This is not a new concept,
back before C++11, people had ways of expressing the same
```
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
C: It's just less pretty
---
C: During the talk, we'll play a game, where I show a small code snippet
and you tell what it prints
Let's give it a quick try
```
for (int i = 1; i <= 10; ++i)
    std::cout << i << '\n';
```
C: I'm amazed to have such a qulified audience
---
### Size of callable *things*
Function pointers
C: Let's start with function pointers
```
void foo(int arg);
auto f = foo;
std::cout << sizeof(f);
```
C: On most platforms it's the size of a pointer, so 8 or 4 byte respectively
---
Lambdas
C: And here?
```
auto f = [](char arg) {};
std::cout << sizeof(f);
```
C: 1 byte, so we already see a diference
```
int val = 3;
auto f = [val](int arg) {};
std::cout << sizeof(f);
```
C: This time it's sizeof(int), usually 4 byte
```
auto f = [](char arg[10]) {};
std::cout << sizeof(f);
```
C: 1 byte.
C: The size of a closure depends soly on the size of what is captured
---
### Function pointer types
```
void foo(int arg);
auto f = foo;
```
C: You guessed it, it's:
```
decltype(f) == void(*)(int);
```
---
C: How about member functions
```
class foo
{
    void bar(int arg);
};
auto f = &foo::bar;
```
C: You guessed it, this is invalid, bar is private
```
class foo
{
public:
    void bar(int arg);
};
auto f = &foo::bar;
```
C: so what's the type of f?
```
decltype(f) == void(foo::* bar)(int)
```
C: now how about calling f?
```
f(3);
```
C: Nope
```
foo obj;
(obj.*f)(3);
```
---
```
auto f = [](int arg) {};
```
C: You guessed right, this was a trick question. Every lambda has a unique type
```
decltype(f) == decltype(f)
```
---
# Alternatives
---
### Coroutines
C: // copy basics from blog post
---
C: Let's benchmark it
D: Switch to visual studio, and run benchmark
C: Why? Coroutines internally use type erasure, again we have to store the state
C somwhere, however this time we are storing both closure state and the
function state
---
C: not all hope is lost, we still have arguably the best C++ feature left
```
int foo() { return 3; }

void bar()
{
    int(*f)() = foo;
    std::cout << (***********/*INLINE COMMENTS YAY*/************f)();
}
```
**Function pointers**
C: If you are confused by what you are looking at,
C: watch James McNeills great lightning talk from this years MeetingC++
---
C: on a more serious note, we'll see that they can be very usefull
```
void(*f)(int) = [](int arg) { std::cout << arg; };
f(3);
```
Lambdas are convertible to function pointers
---
C: problem solved
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

delegate del{ [](int arg) { std::cout << arg; } };
del(3);
```
---
C: With the help of templates we can make it generic
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
C: We are done right? Although there is one small problem. Anyone an idea?
```
int arr[10];
arr[0] = 2;
delegate<void(int)> del{ [arr](int arg) { std::cout << arg + arr[0]; } };
del(1);
```
Only captureless lambdas are convertible to function pointers
C: There is a good reason for that, think back to the size of closures.
C: sizeof(delegate) is the size of one function pointer,
C: where should it store the closure state?
C: The array is larger than the entire delegate class,
C: there is literraly no way of stuffing the information in there
---
C: The solution is simple
# Convert to functional programming
C: Only use pure functions
---
C: Maybe, there is a way constructing a function pointer with a capturing lambda
C: The issue is, that we have to store the closure somewhere
C: Enter the C++'s least ambigious keyword
# static
---
```
static int val = 4;
auto pure = [](int arg)
{
    return arg + val;
};
std::cout << pure(-1);
```
C: pure lambdas aren't that pure after all,
C: this works because the compiler can resolve the address of val at compile time
C: now we can do
```
int(*f)(int) = pure;
```
---
C: With that we can add a new constructor to delegate
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
C: We are golden now, right? Someone see any issues?
C: We havn't talked about copying and moving, but let's do that later
C: We are doing inproper argument type forwarding:
```
invoke_ptr_ = static_cast<invoke_ptr_t>([](Args... args) -> R
{
    return cap(std::forward<Args>(args)...);
});
```
C: To avoid unnecessary moving, it should be
---
```
thread_local static T cap{ std::forward<T>(closure) };

invoke_ptr_ = static_cast<invoke_ptr_t>([](Args&&... args) -> R
{
    return cap(std::forward<Args>(args)...);
});
```
C: But that does not work as that would not fit the function pointer type
---
C: Lets look at this:
```
using del_t = delegate<int(void)>;

std::vector<del_t> vec;
for (int i = 0; i <= 3; ++i)
    vec.emplace_back([i]() { return i; });

std::cout << vec.front()();
```
C: You guessed it, again it prints 3
Although each lambda has a unique type, here there is only **ONE**
lambda, which means that
```
thread_local static T cap{ std::forward<T>(closure) };
```
C: gets created only once, the first time this constructor is called
---
C: Sure we could tell the user to never do that.
C: However this would result in an interface that is:
Easy to use correctly
Easy to use incorrectly
C: When truely what we want is:
---
Easy to use correctly
Hard to use incorrectly
---
# How about we store the closure inplace
---
C: now what should the local varible type be?
```
template<typename R, typename... Args> class delegate<R(Args...)>
{
public:
	using invoke_ptr_t = R(*)(Args...);
    using storage_t = ???
}
```
C: The type of the closure does only become visible in the constructor,
C: as far as I know there is no way of making that type visible to other parts
C: of the class.
C: It seems we may have to resort to the dark art of type less storage
---
```
std::aligned_storage
```
---
C: it is a byte array, with limited size and alignment
´´´
void foo()
{
	std::aligned_storage<sizeof(int), alignof(int)> storage;
	new(&storage)int{ 3 };

	std::cout << reinterpret_cast<int&>(storage);
}

void bar()
{
	foo();
	std::cout << 6;
}
´´´
C: So what does this print?
---
# UB
---
C: Yes it prints 3,
and then it corrupts the stack trying to leave foo()
C: Who can spot the bug?
```
void foo()
{
	std::aligned_storage<sizeof(int), alignof(int)> storage;
	new(&storage)int{ 3 };

	std::cout << reinterpret_cast<int&>(storage);
}

void bar()
{
	foo();
	std::cout << 6;
}
```
---
```
void foo()
{
	std::aligned_storage<sizeof(int), alignof(int)>::type storage;
	new(&storage)int{ 3 };

	std::cout << reinterpret_cast<int&>(storage);
}

void bar()
{
	foo();
	std::cout << 6;
}
```
C: There was ::type missing, by writing into storage, which essentally was an empty
struct, we corrupted the stack
---
C: With that we can change delegate to:
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
C: So, now we successfully stored the closure, but how do we get it back out?
C: Again we have no way of making the type visible to the rest of the class,
C: operator() invokes the function pointer
C: So how about we change the function pointer type
---
```
template<typename T> explicit delegate(T&& closure)
{
	new(&storage_)T{ std::forward<T>(closure) };

	invoke_ptr_ = static_cast<invoke_ptr_t>(
		[](storage_t& storage, Args&&... args) -> R
	{
		return reinterpret_cast<T&>(storage)(std::forward<Args>(args)...);
	});
}
```
C: This works because inside a lambda all current types are visible
C: Note the change to rvalue references for the arguments.
C: That solves our forwarding issue.
---
C: So our entire class now looks like this
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
```
---
C: However, this is no longer a **ZOA**
# ~~ZOA~~
C: If we construct it with a pure function, we'll pay none the less
---
C: How about we split interface and implementation
C: Let's create one implementation per use case
One implementation per use case
* pure
* inplace_triv
* inplace
* dynamic
C: pure, only works with pure functions,
C: inplace_triv stores the closure inplace,
C: only works with trivailly constructable and destructable types.
C: More on why we should make that seperation, later.
C: inplace stores the closure inplace.
C: dynamic stores the closure on the heap
---
C: Both pure and inplace_triv have default copy, move and destruction.
C: With inplace it gets a bit more tricky, we cannot just copy the storage
C: object byte for byte. So agian, same issue how do we get the type out of
C: storage in those operations.
C: let's first look at the destruction
```
~inplace()
{
	// call closure destructor
}
```
---
C: The answer is obviously
# MORE FUNCTION POINTERS
C: Sadly that ain't no joke
---
C: We'll store a second function pointer
```
template<typename T> explicit inplace(T&& closure)
{
    // ... same as before

    destructor_ptr_ = static_cast<destructor_ptr_t>(
		[](storage_t& storage) noexcept -> void
		{ reinterpret_cast<T&>(storage).~T(); }
	);
}
```
---
C: Now we can do
~inplace()
{
	destructor_ptr_(storage_);
}
---
C: Similar story for the copy operations
```
template<typename T> explicit inplace(T&& closure)
{
    // ... same as before

    copy_ptr_ = copy_op<T, storage_t>();
}
```
C: So why put another layer of indirection in there?
---
C: Let's look at the implementation of copy_op
```
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
C: It's fairly straight forward, we write into the local storage object,
C: the closure that is currently stored in the source object
C: But why the enable_if? Let's look a the other overload
---
```
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
C: As you can see, it is just a dummy function tha'll fail on instanciation.
C: Why can't we build a delegate with move only types?
---
C: Consider this example
```
del_t del_a = rand_bool ? copy_move_closure{} : move_only_closure{};

del_t del_b = del_a; // can we copy?
```
C: We would have to make decisions at compile time about run time properties.
C: I see 2 possible solutions if you *really* want move only delegates.
C: You could make a move only delegate, or have a throwing copy.
---
C: The copy constructor is fairly straight forward
```
inplace(const inplace& other) :
    invoke_ptr_{ other.invoke_ptr_ },
    copy_ptr_{ other.copy_ptr_ },
    destructor_ptr_{ other.destructor_ptr_ }
{
    copy_ptr_(storage_, other.storage_);
}
```
C: Note, we have to use the new copy pointer
---
C: The copy assignment is similar
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
C: Notic, this time we have to destroy our closure we before copying
---
C: in order to stay close to std::function's api,
C: the calling operator should be marked const
```
R operator() (Args&&... args) const
{
    return invoke_ptr_(storage_, std::forward<Args>(args)...);
}
```
---
C: Still, we could be modifying storage,
C: so we have to mark storage as mutable
```
private:
	mutable storage_t storage_;

	invoke_ptr_t invoke_ptr_;
	copy_ptr_t copy_ptr_;
	destructor_ptr_t destructor_ptr_;
```
---
# Designing the interface
---
Why did we even split interface and implementation?
C: Initially I didn't.
C: There were multiple constructors and flags.
C: After a while is was coplicated to keep track of all the posibilites,
C: due to all the conditionality involved,
C: depending on what it was constructed with.
How do we bring it all together?
---
# Variant
C: A new C++17 feature, based on an old concept.
C: It is basically an *or* rather than an *and* struct.
C: I'll not go into to much detail here, if you want to know more,
C: There is a good talk from this years cppcon called:
C: "Variants: Past, Present, and Future" by David Sankel.
---
C: Let's see it in action
D: Show on godbolt std::function version,
D: then variant version,
D: then show just calling pure
C: Not only is it not **ZOA** anymore,
C: its not even better than plain std::function.
---
C: At that point I was somewhat hitting a wall.
Was all that for nothing?
Is there just no better way of solving this problem?
C: I was frustrated, it all looked so good,
C: but again and again bugs forced me to abandon my previous solutions.
C: After a couple of days, filled with the fruitless endevor of,
C: saving a sunk ship, I finally relized:
C: Yes there is no better way of solving this problem,
---
# I was trying to solve the wrong problem
---
C: What did we say earlier about C++? Right.
# **C++** you only pay for what you use
C: But we have no idea what the user will use.
C: Let's make the user tell us what he wants.
C: You tell us what you want, and that's what you'll get.
C: No background magic trying to figure out what the user wants.
---
C: Here is what it looks like
```
template<
	typename T,
	template<size_t, typename, typename...> class Spec = spec::inplace,
	size_t size = detail::default_capacity
>
class delegate; // unspecified

template<
	typename R, typename... Args,
	template<size_t, typename, typename...> class Spec,
	size_t size
>
class delegate<R(Args...), Spec, size>;
```
C: We let the user decide what kind of limitations they want.
C: And we even provide a default.
---
D: Show how it works on godbolt
---
C: I'd like a world where people are ok with no empty constructor,
C: it results in cleaner code.
C: However it seems people want empyt construction,
C: and if called empty it should explode nicely.
C: We could just add a boolean flag, empty.
```
R operator() (Args&&... args) const
{
    if (empty)
        throw std::bad_function_call();

    return invoke_ptr_(storage_, std::forward<Args>(args)...);
}
```
C: That's not nice. Maybe there is better way.
---
What excatly are we building here?
C: We are storing function objects, so why not store a thrwoing function?
C: Said function
```
template<
	typename R,
	typename S,
	typename... Args
> static R empty_inplace(S&, Args&&... args)
{
	throw std::bad_function_call();
}
```
---
C: And the empty constructor
```
explicit inplace() noexcept :
    invoke_ptr_{ empty_inplace<R, storage_t, Args...> },
    copy_ptr_{ copy_op<std::nullptr_t, storage_t>() },
    destructor_ptr_{ [](storage_t&) noexcept -> void {} }
{
    new(&storage_)std::nullptr_t{ nullptr };
}
```
C: That's all we have to change.
---
C: Making checking for emptyness equally simple.
```
bool empty() const noexcept
{
	return reinterpret_cast<std::nullptr_t&>(storage_) == nullptr;
}
```
C: That's it. Sure we glossed over a lot of details.
C: None the less, those are all the important pieces you need.
---
# Benchmarks
Benchmark with and without small size optimization

# Lessons learned
By seperating interface from implementation, you get more freedom and if one implementation
works and the other does not. You know its not an interface issue.

Test drived development

---
C: Did you notice, we barely used any controll flow statements?
C: In this entire 600 line, implementation there are only 2 ifs.
C: The ones in the inplace copy and move assignments.
C: That's it. No switch, no loops.
C: Why is that?
# Type oriented design
C: Fundamentally, every program is some sort of data transformations.
C: Take data, transform it and put it somewhere.
C: Ask yourself, what is a type?
C: At its core, a type describes a memory layout,
C: so what is more ideal than a type, for describing data transformations?
---

---
# Questions
C: I'll start
---
How do you know it works?
C: I don't. However, this may convince you.
D: Show IncludeOS server.

### Links:
[James McNellis - "my favorite C++ feature"](https://youtu.be/6eX9gPithBo)
[David Sankel - "Variants: Past, Present, and Future"](https://youtu.be/k3O4EKX4z1c)
[Full implementation](https://github.com/hioa-cs/IncludeOS/blob/dev/api/util/delegate.hpp)

---
# Hiring?
C: Shameless plug, if you are currently hiring,
feel free to talk to me. // better wording
