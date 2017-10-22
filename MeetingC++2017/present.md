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
    std::function<bool(const T&, const B&)> compare
  );
}
```

Note:
- `std::max` overload
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

If it's a free function, using templates is basically always a better option.
No one would write code like this, right?

Class level: here we decide the memory layout
Constructor level: here we decide how the memory layout is initialized
Run time: here we potentially mutate the memory

I won't show you any benchmarks in this talk. Benchmarks, especially about
this topic can be misleading.
Implementation can limit inlining.
Implementation can cause heap allocation.
Alternate implementations, like inline storage can make iteration worse.

Note how I always say can, not will. Using run time polymorphism,
will make your performance much less predictable.

Point to Louis' talk to learn more about alternate implementations.
