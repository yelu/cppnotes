# Type Traits

Type Traits是模板编程的重要技术之一，它完全工作在编译时，以模板类的形式存在，主要提供两类操作。

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

这些模板类有什么用处呢？答案是让编译器根据不同的数据类型选择不同的代码路径。例如，我们有一个函数`algorithm`封装了某种算法，当数据类型分别为`int`和`float`时候需要两种完全不同的实现版本，可是对外我们想提供统一的模板类，将类型的选择和切换封装在内部。这就是Type Traits发挥作用的时候。

抛开标准库已经内置的常见Type Traits不用，自己实现满足以上要求的Type Traits也并不复杂。

首先，我们需要定义两个辅助类型，帮助`int_or_float`这个Type Traits区分不同的类型。

```cpp
struct IntFlag {};
struct FloatFlag {};

template<typename T>
struct int_or_float {
   typedef FloatFlag type;
};

template<>
struct int_or_float<int> {
   typedef IntFlag type;
};
```

然后，我们通过模板和重载实现代码路径的选择。上面定义的辅助类型`IntFlag`和`FloatFlag`将作为函数的额外模板参数，以区分不同版本的函数。

```cpp
template<typename T>
void algorithm(T num, IntFlag)   { /*...*/ } 

template<typename T>
void algorithm(T num, FloatFlag) { /*...*/ } 

template<typename T>
void algorithm(T num) {
   algorithm(num, int_or_float<T>::type())
} 
```

可以看到，Type Traits是我们实现编译时代码分发(dispatcher)功能的核心技术。

## Type Transformations

第二类是对类型`T`做某种操作生成其它类型。
