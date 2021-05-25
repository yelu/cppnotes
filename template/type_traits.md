# Type Traits

Type Traits是模板编程的重要技术之一，完全工作在编译时，以模板类的形式存在，主要提供两大类操作：

* 查询类型T的属性
* 基于类型T变换出新类型

## Type Traits

第一类是检查类`T`是否拥有一些特征或属性，这些模板类直接就称为Type Traits。例如，T是一个int类型吗？是一个class类型吗？是申明为const了吗？


```cpp
#include <type_traits>

std::cout << std::is_integral<int>::value << '\n';
std::cout << std::is_integral<float>::value << '\n';

/*
Output:
true
false
*/
```

这些模板类有什么用处呢？答案是让编译器根据不同的数据类型选择不同的代码路径，称为Tag Dispatching。

例如，我们有一个函数`algorithm`封装了某种算法，当数据类型分别为`int`和`float`时候需要两种完全不同的实现版本，可是对外我们想提供名字统一的算法，将类型的选择和切换封装在内部。这就是Type Traits发挥作用的时候。抛开标准库已经内置的常见Type Traits，自己实现满足以上要求的Type Traits也并不复杂。

首先，我们需要定义两个辅助类型，帮助`int_or_float`这个Type Traits针对不同的类型做分发。

```cpp
struct IntTag {};
struct FloatTag {};

template<typename T>
struct is_int {
   typedef FloatTag type;
};

template<>
struct is_int<int> {
   typedef IntTag type;
};
```

然后，我们通过函数的参数类型重载实现代码路径的选择。上面定义的辅助类型`IntTag`和`FloatTag`将作为函数的额外参数，以区分不同版本的函数。

```cpp
template<typename T>
void algorithm_impl(T num, IntTag)   { /*...*/ } 

template<typename T>
void algorithm_impl(T num, FloatTag) { /*...*/ } 

template<typename T>
void algorithm(T num) {
   algorithm_impl(num, is_int<T>::type())
} 
```

C++ 17种引入的编译时条件判断（compile-time if），进一步满足了根据类型做代码分发的这类需求。

```cpp
template<typename T>
struct is_int {
   static constexpr bool value = false;
};

template<>
struct is_int<int> {
   static constexpr bool value = true;
};

template <typename T>
int algorithm(T t) {
     if constexpr (is_int<T>::value) { ... }
     else { ... }
}
```

## Type Transformations

第二类是对类型`T`做某种操作生成其它类型。
