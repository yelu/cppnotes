# 内部链接与外部链接

链接作为构建可执行程序的最后一步，最令人困扰和恼火的错误多是在这一步产生的。为什么编译器会报符号没有定义？重复定义又是什么意思？为了定位、理解其中的原因，必须首先了解符号链接的作用域，是内部的还是外部的。

## 声明(Declaration)和定义(Definition)

声明的作用是将一个符号(类型、实例、函数)引入一个作用域，等同于宣称：随后的代码需要使用符号A，此处是符号A的“规格说明”，例如名字、类型、签名等等。

有些声明被重复多次是必要的、合法的，例如类的前向声明可能被多个头文件同时使用。而另一些则没必要、不合法，例如类中同一个成员声明两次。

```cpp
int foo(int,int);    // 函数前置声明
typedef int Int;     // typedef声明
extern int g_var;    // 外部引用声明
class bar;           // 类前置声明
using std::cout;     // 名字空间引用声明
friend test;         // 友员声明
class foo
{
    static int i; // 类静态成员声明
    int j;  // 类成员声明
};
```

定义是一个符号在作用域内的唯一存在，在同一作用域中不可重复定义同名符号。

```cpp
int y;
static int i;
extern int i = 10; // 有初始化值的extern被认为是定义
const double PI = 3.1415;
void test(int p) {};
class Foo {...};
Foo a; // class Foo {...};
Bar b; // struct Bar {...};
enum Color {RED, GREEN, BLUE};
union Rep {...};
```

C++将声明和定义区分开来的原因是，允许将源代码拆成多个源文件。而它们之间最终需要“交流”，即需要使用彼此定义在源文件中的符号，声明就是相互间“沟通”和引用彼此符号的方式。

声明作为“规格说明”可以理解为一种“引用或者指针”，而定义才是“本体”。因此，在链接阶段对符号的关注应该放在定义上面，定义的可见性、所用域是什么？能否重名？这一问题就是所谓的“内部链接和外部链接”。

## 内部与外部链接

如果一个符号对于它的编译单元来说是局部的，即在链接时不会与其它编译单元中的同名符号相冲突，那么这个符号有内部链接。以下定义有内部连接:

* 静态自由函数的定义。自由指既不是类的成员，也不是友元
* 静态自由变量的定义
* 静态友元函数的定义
* class定义
* enum定义
* union定义
* inline函数定义。包括自由函数和非自由函数
* const常量定义

如果一个符号在链接时可以和其它编译单元交互，那么这个符号就有外部连接。以下定义有外部链接:

* 非静态自由函数的定义
* 非静态自由变量的定义
* 非静态友元函数的定义
* 类的成员函数的定义。包括一般成员函数、静态成员函数、虚函数，inline除外
* 类的静态成员变量定义

C++支持通过链接属性关键字来指定一个符号要不要从该编译单元中暴露给外部。这些关键字包括`extern`和`static`：

* extern，外部链接。该符号来自于其它编译单元。
* static，内部链接。该符号只属于自己当前的编译单元，其它编译单元中即使有同名符号，它们仍然是两个不相关的符号。

```cpp
// File: common.cpp
// var1的定义具有外部链接，是全局唯一的
int var1 = 10;
// var2的定义具有内部链接，其它源文件仍然可以定义自己的var2
static int var2 = 20;

// File: a.cpp
// 声明使用其它源文件定义的var1
extern int var1;
```

当`a.cpp`要用到另一个`common.cpp`中定义的符号var1时（函数同理），就需在`a.cpp`中**声明符号var1**，让编译顺利进行下去。待到链接阶段，链接器会去其它目标文件（common.o）中**查找符号var1的定义**，最终产出可执行程序。期间，出错的结果有两种：没有任何地方定义了var1，或者有多个地方定义了va1，分别代表了两种最常见的链接错误：Unresolved Symbol和Symbol Redefination。

## 练习

**1.** C++的代码文件分为头文件(.h/.hpp)和源文件(.cpp/.cxx/.c/.cc)。哪些内容应该写在头文件里，哪些应该写在源文件里，分别会导致怎样的链接结果，和符号链接问题紧密相关。在下面的例子中，头文件`common.h`中定义了一个变量`int a`，当它被多个源文件包含时，编译时会有错误吗？链接时会有错误吗？解决的方法是什么？

```cpp
// File: common.h
int a;

// File: a.cpp
#include "common.h"

// File: b.cpp
#include "common.h"
```

**2.** `b.cpp`中`set_var_from_b`对`var`的更改，在`a.cpp`中无法通过`get_var_from_a`获取到，为什么？如果想要获取到，需要怎么做？

```cpp
// File: common.h
static int var = 0;

// File: a.cpp
#include "common.h"
int set_var_from_a(int val) { var = val; }
int get_var_from_a() { return var; }

// File: b.cpp
#include "common.h"
int set_var_from_b(int val) { var = val; }
int get_var_from_b() { return var; }
```
