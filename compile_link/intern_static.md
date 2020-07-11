# 内部链接与外部链接

链接作为构建可执行程序的最后一步，最令人困扰和恼火的错误多是在这一步产生的。比如，两种最常见的链接错误：Unresolved Symbol和Symbol Redefination。为了理解其中的原因，必须了解符号链接的作用域，是内部的还是外部的。

## 声明(Declaration)和定义(Definition)

声明的作用是将一个符号(类型、实例、函数)引入一个作用域，等同于宣称：随后的代码需要使用符号A，此处是符号A的“规格说明”，例如类型、名称、签名等等。

有些声明被重复多次是必要的、合法的，例如类的前向声明。而另一些则没必要、不合法，例如类中同一个成员声明两次。

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

定义是一个符号在作用域内的唯一存在，在同一作用域中不可重复定义同名符号。声明作为“规格说明”是一种“表象”，而定义才是“本体”，占用实实在在的存储空间。

```cpp
int y;
static int i;
extern int i = 10; // 有初始化值的extern被认为是定义
const double PI = 3.1415;
void test(int p) {};
Foo a; // class Foo {...};
Bar b; // struct Bar {...};
enum Color {RED, GREEN, BLUE};
union Rep {...};
```

C++将声明和定义区分开来的原因是，允许将源代码拆成多个源文件，而它们之间最终需要“交流”，即需要使用定义在别的源文件中的符号，声明就是相互间沟通和引用彼此符号的方式。

## 内部与外部链接

如果一个符号对于它的编译单元来说是局部的，即在链接时不会与其它编译单元中的同名符号相冲突，那么这个符号有内部链接(注：声明的链接属性并不重要，无需讨论)。以下情况有内部连接:

* 静态自由函数（既不是类的成员函数，也不是友元函数）、静态友元函数、静态变量的定义（类的静态成员除外）
* enum定义
* union的定义
* inline函数定义（包括自由函数和非自由函数）
* 类的定义
* const常量定义

如果一个符号在链接时可以和其它编译单元交互，那么这个符号就有外部连接。以下情况有外部链接:

* 类非inline函数总有外部连接。包括一般类成员函数、静态成员函数、虚函数。
* 类静态成员变量总有外部连接。
* 非静态自由函数、非静态友元函数及非静态变量。

C++支持通过链接属性关键字来指定一个符号要不要从该编译单元中暴露给外部。这些关键字包括`extern`和`static`：

* extern，外部链接。该符号来自于其它编译单元。
* static，内部链接。该符号只属于自己当前的编译单元，其它编译单元中即使有同名符号，它们仍然是不相关的两个符号。

当`a.cpp`要用到另一个`b.cpp`中定义的符号B（变量、函数等）时，就需在`a.cpp`中声明符号B，让编译顺利进行下去。待到链接阶段，链接器会去其它目标文件（.o/.obj）中查找符号B，最终产出可执行程序。期间，出错的结果有两种：没有任何地方定义了符号B，或者有多个地方定义了符号B。分别代表了两种最常见的链接错误：Unresolved Symbol和Symbol Redefination。

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
