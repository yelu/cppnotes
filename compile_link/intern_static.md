# 内部链接与外部链接

C++的代码文件分为头文件(.h/.hpp)和源文件(.cpp/.cxx/.c/.cc)。哪些内容应该写在头文件里，哪些应该写在源文件里，分别会导致怎样的链接结果，和符号链接问题紧密相关。

在下面的例子中，头文件`common.h`中定义了一个变量`int a`，当它被多个源文件包含时，就会导致重复定义的链接错误。

```cpp
// File: common.h
int a;

// File: a.cpp
#include "common.h"

// File: b.cpp
#include "common.h"
```

C++语句分为申明和定义。符号可以被多次申明，但只能被定义一次。

## 声明(Declaration)

声明将一个名称引入一个作用域。一个声明被重复多次有时是合法且必要的，例如类的前向声明。而在另一些时候则是不合法且没必要的，例如在类中同一个成员申明两次。

```cpp
int foo(int,int);    // 函数前置声明
typedef int Int;     // typedef 声明
extern int g_var;    // 外部引用声明
class bar;           // 类前置声明
using std::cout;     // 名字空间引用声明
friend test;         // 友员声明
// 类成员函数及静态数据成员的声明不能重复
class foo
{
    static int i;
    static int i; // wrong
    int foo();
    int foo(); // wrong
};
```

## 定义(Definition)

定义提供一个实体(类型、实例、函数)在一个作用域的唯一描述。在同一作用域中不可重复定义一个实体。

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

## 内部与外部链接

当`a.cpp`要用到另一个`b.cpp`中定义的符号A（变量、函数等）时，就需在`a.cpp`中申明符号A。链接器会在链接阶段去其它的目标文件（.o/.obj）中查找符号A，出错的结果有两种：没有任何地方定义了符号A，或者有多个地方定义了符号A。分别代表了两种最常见的链接错误：Unresolved Symbol和Symbol Redefination。

如果一个符号对于它的编译单元来说是局部的，并且在链接时不会与其它编译单元中的同名符号相冲突，那么这个符号有内部连接(注：有时也将声明看作是无连接的，这里我们统一看成是内部连接的)。以下情况有内部连接:

* 所有的声明
* 静态自由函数（既不是类的成员函数，也不是友元函数）、静态友元函数、静态变量的定义（类的静态成员除外）
* enum定义
* union的定义
* inline函数定义（包括自由函数和非自由函数）
* 类的定义
* const常量定义

如果一个符号在链接时可以和其它编译单元交互，那么这个符号就有外部连接。以下情况有外部连接:

* 类非inline函数总有外部连接。包括一般类成员函数、静态成员函数、虚函数。
* 类静态成员变量总有外部连接。
* 非静态自由函数、非静态友元函数及非静态变量。

C++支持通过链接属性关键字来指定一个符号要不要从该编译单元中暴露给外界。这些关键字包括`extern`和`static`：

* extern，外部链接。除了本编译单元，其它编译单元也能够访问。
* static，内部链接。只属于自己当前的编译单元。
