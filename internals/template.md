# 模板

## Variadic Template

可变参数模板是C++11中引入的新特性。在这之前，定义一个支持可变参数个数的模板函数需要进行枚举：

```C++
template<typename T1>
std::string
format_str(const char* format, const T1& t1);

template<typename T1, typename T2>
std::string
format_str(const char* format, const T1& t1, const T2& t2);
```

使用可变参数模板时，通常还会结合利用编译器进行递归展开（meta pragramming常用技巧）。事实上，编译器帮助我们生成了一系列函数，它们分别接收不同个数的模板参数。

```C++
template<typename T>
std::string
format_str(boost::format& format, const T& t)
{
    return format % t;
}

template<typename T, typename... Rest>
std::string
format_str(boost::format& format, const T& t, const Rest&... rest)
{
    std::string tmp = format % t;
    return format_str(tmp, rest...);
}
```

## Traits and Policies

Traits和Policies在一般C++业务代码中并不常见，通常也不需要涉及。而在Template Programming中却被大量使用，在C++标准库中随处可见。

### Traits

Trait通常是一个模板类，定义了获取有关一种或者多种Type的描述信息的接口，即定义了**Type Interfaces**。这样做的好处之一是使用trait的模板类可以统一处理内置类型和自定义类型。

例如，如果我们需要根据参数的不同类型来调用同名函数的重载版本，就可以[使用Trait实现](https://www.oreilly.com/library/view/c-in-a/059600298X/)。

```cpp
// Type trait to test whether a type is an integer.
struct is_integer_tag {};
struct is_not_integer_tag {};
// The default is that a type is not an integral type.
template<typename T>
struct is_integer {
 typedef is_not_integer_tag tag;
};
// Override the default explicitly for all integral types.
template<>
struct is_integer<int> {
 typedef is_integer_tag tag;
};

// Constructor uses the is_integer trait to distinguish integral from nonintegral
// types and dispatches to the correct overloaded construct function.
template<typename T, typename A>
template<typename InputIter>
list<T,A>::list(InputIter first, InputIter last)
{
 construct(first, last, is_integer<InputIter>::tag());
}

// The construct member functions finish the initialization of the list. The
// integral version casts the arguments to the size and value types.
template<typename T, typename A>
template<typename InputIter>
void list<T,A>::construct(InputIter first, InputIter last,
 is_integer_tag)
{
 insert(begin( ), static_cast<size_type>(first),
 static_cast<T>(last));
}
// The non-integral version copies elements from the iterator range.
template<typename T, typename A>
template<typename InputIter>
void list<T,A>::construct(InputIter first, InputIter last,
 is_not_integer_tag)
{
 insert(begin( ), first, last);
}
```

### Policies

Polices通常以类模板的模板参数形式提供。它定义了一些函数接口，并向模板类注入实现，即定义了**Function Interfaces**。注入方式或将Policy作为基类继承，或将一个Policy实例化作为成员属性。

通过提供多个Plocies模板类型，还可以将多种不相关的Plocies分开定义和实现。例如，unordered_map就通过继承提供了Hasher和Alloc两个Polices，一个用于提供Hash实现，另一个用于管理内存分配。

```cpp
template<
    class Key,
    class T,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>,
    class Allocator = std::allocator< std::pair<const Key, T> >
> class unordered_map;
```

使用Plocies而不是Virtual Functions的原因，和使用模板还是虚函数来实现多类型系统的理由一致，一个发生在编译时，一个发生在运行时。

### Traitsh和Policies的交互

Data和Function的交互是一个类的核心所在。因此，Traits和Polices是相互密切联系的。

模板库通过剥离对Type和Policies接口的实现，可以极大地统一模板类本身的代码逻辑。

## SFINAE and enable_if

[[link]](https://eli.thegreenplace.net/2014/sfinae-and-enable_if/)[[cache]](SFINAE_and_enable_if.html)

SFINAE(Substitution Failure Is Not An Error) has proved so useful that programmers started to explicitly rely on it very early on in the history of C++. One of the most notable tools used for this purpose is enable_if. It can be defined as follows:

```cpp
template <bool, typename T = void>
struct enable_if
{};

template <typename T>
struct enable_if<true, T> {
  typedef T type;
};
```

And now we can do things like this:

```cpp
template <class T,
         typename std::enable_if<std::is_integral<T>::value,
                                 T>::type* = nullptr>
void do_stuff(T& t) {
  std::cout << "do_stuff integral\n";
    // an implementation for integral types (int, char, unsigned, etc.)
}

template <class T,
          typename std::enable_if<std::is_class<T>::value,
                                  T>::type* = nullptr>
void do_stuff(T& t) {
    // an implementation for class types
}
```
