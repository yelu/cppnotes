# SFINAE and enable_if

当编译器遇到一个函数调用时，会根据实际的参数类型找到哪个同名函数才是应该调用的函数。如果这其中涉及到模板函数，就需要对模板参数进行实例化，即所谓的类型替换（substitution）。尽管最终匹配的结果可能是，某个模板函数并不是符合要求的函数，但是这些同名模板依然需要被分析，原因是：

* 只有实例化之后，才能判断当前模板函数是否符合要求。
* 如果有多个语法正确的匹配结果，需要选出最优匹配。因此，所有可能的匹配都需要先去尝试，而不是直接放弃实例化。

```cpp
int negate(int i) {
  return -i;
}

template <typename T>
typename T::value_type negate(const T& t) {
  return -T(t);
}
```

在以上代码中，考虑调用`negate(42)`。为了找到最优匹配，编译器会尝试模板和非模板两个版本。在尝试模板时，推导出的函数声明如下，显然不合法，因为`int`类型没有`value_type`这个成员属性，最终匹配到的调用是非模板实现。在这个过程中，虽然发生了模板实例化，并且最终失败，但是编译器不会输出任何的警告或者错误，编译器的这一行为被称为SFINAE（Substitution Failure Is Not An Error）。

```cpp
int::value_type negate(const int& t);
```

## std::enable_if

SFINAE不太容易被理解的地方倒不是其行为本身，而是编译器的这一行为给人的感觉是，它应该是自然而然的，不至于被总结成一个feature并且冠以一个乍看起来很“高深”的名字，我们甚至不知道SFINAE该怎么发音。

实际上，开发者作为语言的用户，而非编译器作者，确实也不必要花费过多的精力再去纠结这些问题，认识到它的行为就足够了。

SFINAE最常被人津津乐道的应用是用来实现std::enable_if，也就是根据不同的参数类型来实例化不同的模板。

```cpp
template <bool, typename T = void>
struct enable_if
{};

template <typename T>
struct enable_if<true, T> {
  typedef T type;
};

// an implementation for integral types (int, char, unsigned, etc.)
template <class T,
         typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
void do_stuff(T& t) { }

// an implementation for class types
template <class T,
          typename std::enable_if<std::is_class<T>::value, T>::type* = nullptr>
void do_stuff(T& t) { }
```

std::enable_if其实和Type Traits的应用场景是高度重合的，可以回顾一下Type Traits相关的内容来对比理解。具体来说，tag dispatching可以作为std::enable_if的替代方案。而由于std::enable_if和tag dispatching二者在代码可读性方面做的并不好，在可用的语言标准足够高的时候，compile-time if通常是更好的选择。

## References

* [SFINAE and enable_if](https://eli.thegreenplace.net/2014/sfinae-and-enable_if/) [[cache]](ref/SFINAE_and_enable_if.html)
* [Notes on C++ SFINAE, Modern C++ and C++20 Concepts](https://www.bfilipek.com/2016/02/notes-on-c-sfinae.html) [[cache]](ref/notes_on_c_SFINAE.html)