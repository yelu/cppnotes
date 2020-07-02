# 类型

C++的基本数据类型有7种(int/float/double/char/wchar_t/bool/void)，其中int、double和char还可以被合法添加一种或多种修饰符(signed/unsigned/short/long)。这些类型共同的特点是“简单”，在内存中占据连续的1-8个字节，任何对它们的操作都显得干干净净、清清楚楚。

除了简单数据类型，C++引入了自定义类型，也就是类(Class)。通过组合基本类型和/或一个类，可以定义出另一个新类。

类的很多复杂行为都可以归结于一个动机之上：**C++想要自定义类型表现得和基本数据类型一样，让它们平起平坐**。例如，可以等号赋值。奈何类远比基本类型复杂得多，现实和理想中间有一条的鸿沟，填补它的正是类的六大内置函数：


|Member function | typical form for class C|
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
