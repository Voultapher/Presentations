std::function is not zoa

C++, you pay for what you usage

Explaing what a closure is

Well play a game, I show a small code snippet and you tell what it prints

Easy example numbers 1-10

Print example: aligned_storage without ::type, have it be 2 functions,
it'll print garbadge and stack corrupts when trying to leave first function

Show Coroutine approach with benchmarks

Show function pointer conversion, solution: convert to funcitonal programming

How make std::function interface, show template specialization

Show static solution, sol
Only one small issue.
```
using del_t = delegate<int()>;
std::vector<del_t> v;

for (int i = 0; i < 10; ++i)
    v.emplace_back([i]{ return i; });

int val = v.front()();
```
You guessed it val is 10;
We can only have exactly one delegate per delegate type value type pair.
Sure we could tell the user to never do that.
However this would result in an interface that is:
    Easy to use correctly
    Easy to use incorrectly

When truely what we want is:
    Easy to use correctly
    Hard to use incorrectly

data layout by design, programms are data transformations -> types describe data lyouts


I realized, I was trying to solve the wrong problem

Pick up: C++ you pay for what you use and explain limiting interface

How do you know it works? I don't. However its deployed in IncludeOS,
maybe that'll convince you it is somewhat stable.

I'd like a world where people are ok with no empty constructor,
it results in cleaner code.
However it seems people want empyt construction, and if called empty it should explode nicely.

By seperating interface from implementation, you get more freedom and if one implementation
works and the other does not. You know its not an interface issue.

Why can't we build a delegate with move only types. Because we would have to make decisions at compile time about run time properties.

Somewhere put the example:
```
del_t del_a = copy_move_closure;

if (rand_bool)
    del_a = move_only_closure;

del_t del_b = del_a; // can we copy?
```

2 options
You could make a move only delegate, or have a throwing copy.

Be very carefull when having a template<typename T> class_name(T&& val) constructor, it may catch stuff you don't expect or want it to catch
