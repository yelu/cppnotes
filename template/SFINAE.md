# SFINAE and enable_if

SFINAE(Substitution Failure Is Not An Error) has proved so useful that programmers started to explicitly rely on it very early on in the history of C++. One of the most notable tools used for this purpose is enable_if. It can be defined as follows:

```cpp
template <bool, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
  typedef T type;
};
```

And now we can do things like this:

```cpp
// an implementation for integral types (int, char, unsigned, etc.)
template <class T,
         typename std::enable_if<std::is_integral<T>::value,
                                 T>::type* = nullptr>
void do_stuff(T& t) { }

// an implementation for class types
template <class T,
          typename std::enable_if<std::is_class<T>::value,
                                  T>::type* = nullptr>
void do_stuff(T& t) { }
```

## References

* SFINAE and enable_if [[link]](https://eli.thegreenplace.net/2014/sfinae-and-enable_if/)[[cache]](SFINAE_and_enable_if.html)