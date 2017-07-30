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
