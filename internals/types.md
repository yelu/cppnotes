# 类型

C++的基本数据类型有7种(int/float/double/char/wchar_t/bool/void)，其中int/double/char还可以合法地有限组合修饰符(signed/unsigned/short/long)。除了基本类型，C++引入了自定义类型，也就是类(Class)。通过组合基本类型和类，可以定义出一个新类。

基本类型有一个共同的特点是“简单”，在内存中占据连续的1-8个字节，任何操作都显得清清楚楚、干干净净。

类的很多复杂行为都可以归结于一个动机之上：C++想要自定义类型表现得和基本数据类型一样，让它们平起平坐，例如可以在栈上构造，可以用等号赋值。对比Java/Python/C#等语言，它们都不支持在栈上构造自定义类对象，对象等号赋值也不是拷贝语义。基于不同的设计目标和一些现实的考量，这些语言做出了不同的选择。

```cpp
// basic type
int i1 = 1;
int i2 = i1;

// class type
Type t1;
Type t2 = t1;
```

C++值类型的设计看起来自然而然，实现上反而不是那么简单，尤其是还要兼顾面向对象特性和执行效率的时候，类变得比基本类型复杂得多，理想跟现实之间出现了一条鸿沟。C++标准中大量的篇幅都直接或间接因此而来，其中就包括类的一些内置函数的设计和实现。

## 初始化

对于基本类型，用简单的等号赋值就可以完成初始化。而类由于可能包含复杂数据成员，需要一个函数来专门负责此事，这个函数就是构造函数。

```cpp
class C {
public:
    C::C();
    C::C(int a);
}

C c;
C c(2);
```

为了用已经存在的对象构造另一个新对象，C++提供了专门的函数来帮助用户介入构造流程，这个函数就是拷贝构造函数。如果用户没有提供，默认执行Member Wise Copy逐个拷贝数据成员。

```cpp
class C {
public:
    C::C (const C&);
}

C c1;
C c2 = c1;
```

## 销毁

当不再需要某个对象的时候，也不能像基本数据类型那样无需任何额外操作。用户需要提供析构函数用于清理对象持有的资源。

```cpp
class C {
public:
    C::~C();
}
```

## 拷贝

如果把一个已经存在的对象赋值给另一个已经存在的对象，会调用类的等号操作符(=)重载函数。如果用户没有提供，默认执行Member Wise Copy逐个拷贝数据成员。

```cpp
class C {
public:
    C& operator= (const C&);
}
```

## Move语义

栈上值类型带来的复杂性还没有结束。下面的函数`func`返回一个`string`对象，在没有优化的情况下，C++需要在函数内创建变量`s`，然后把它拷贝到`ret`中，最后销毁`s`。

```cpp
string func() {
    string s;
    // do something with s
    return s;
}

string ret = func();
```

实际上，在执行到返回值赋值的时候，已经没有机会再拿到函数的局部变量`s`，因此`s`最后变成什么根本不重要，更聪明的做法是直接交换`ret`和`s`内部数据成员的地址指针。C++11引入的Move语义动机就是避免诸如此类对象拷贝产生的开销：对于右值对象，用户代码没有操作句柄，可以通过直接交换内存指针安全地完成构造或赋值。

```cpp
class C {
public:
    C::C (C&&);
    C& operator= (C&&);
}
```

光是构造、销毁和拷贝，C++就提供了至少6个函数供不同情况下调用。如果用户不提供，还有一些默认的实现会被触发。复杂性就这样一点点积累起来了。

| Member Function | Typical Form for Class C|
|--|--|
| Default constructor | C::C(); |
| Destructor | C::~C(); |
| Copy constructor | C::C (const C&); |
| Copy assignment | C& operator= (const C&); |
| Move constructor | C::C (C&&); |
| Move assignment | C& operator= (C&&); |

## 值类型和引用类型

栈上值类型在C++历史上被摆在很重要的位置，STL就完全是基于值类型构建的。值类型带来的思考负担引起了很多人的不适，尤其是对性能要求苛刻、对编译优化细节刨根问底的开发者。这些负担在使用指针的时候是完全不存在的，因此，使用指针类型去替代值类型也是十分常见的设计模式。

```cpp
// use value type
std::vector<C> cs;

// use reference type
std::vector<C*> cs;
std::vector<std::shared_ptr<C*>> cs;
```
