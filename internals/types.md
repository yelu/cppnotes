# 类型

C++的基本数据类型有7种(int/float/double/char/wchar_t/bool/void)，其中int/double/char还可以合法组合修饰符(signed/unsigned/short/long)。这些类型共同的特点是“简单”，在内存中占据连续的1-8个字节，任何操作都显得清清楚楚、干干净净。

除了基本类型，C++还引入了自定义类型，也就是类(Class)。通过组合基本类型和类，可以定义出一个新类。

类的很多复杂行为都可以归结于一个动机之上：C++想要自定义类型表现得和基本数据类型一样，让它们平起平坐，例如可以用等号赋值、可以在栈上构造。这些看来自然而然的行为，其实并不那么简单。对比Java/Python/C#代码，它们支持在栈上构造对象吗？等号赋值会拷贝一个对象吗？答案是不支持、不会。

```cpp
// basic type
int i1 = 1;
int i2 = i1;

// class type
Type t1;
Type t2 = t1;
```

由于类远比基本类型复杂得多，理想跟现实之间出现了一条鸿沟，C++标准中大量的篇幅都直接或间接因此而来，其中就包括类的一些内置函数的设计和实现。


| Member Function | Typical Form for Class C|
|--|--|
| Default constructor | C::C(); |
| Destructor | C::~C(); |
| Copy constructor | C::C (const C&); |
| Copy assignment | C& operator= (const C&); |
| Move constructor | C::C (C&&); |
| Move assignment | C& operator= (C&&); |

## 初始化

## 销毁

## 拷贝
