# 指针

C和C++是离系统硬件比较近的语言，有直接操纵内存资源的能力，指针便成为学习过程中必不可少的知识点。它提供了一种通过地址变量访问数据的机制，正是这点特殊的功能让理解指针变得困难。

多年前，一位无名人士留下了一份优秀的指针指导手册[《让你不再害怕指针》](ref/pointers_no_more_fear.pdf)。其中指出，理解指针的重点是区分以下核心概念，别把它们混为一谈。本节将以精简的篇幅截取其中最重要的知识点，加以回顾。

* 指针的类型
* 指针指向数据的类型
* 指针的值

## 指针的类型

指针变量首先是一种有类型的变量，和整形、自定义类型没有本质区别，“附带支持”通过地址存取它指向的数据。这里，我们把“对共性的认识”置于“区别性认识”之前。

像定义int等变量一样，定义指针变量也是`数据类型+变量名`的形式。一个简单的原则是，申明语句中去掉指针名，就是指针的类型。

```cpp
// TYPE: int, VARIABLE: a, a是一个整形变量
int a;

// TYPE: int*, VARIABLE: p, p是一个整形指针变量
int* p;

// TYPE: void *(int), VARIABLE: p, p是一个函数指针变量
void (*p)(int)
```

## 指针指向数据的类型

使用指针最终的目的是要访问它指向的数据，因此对其指向数据的类型的认识也是必不可少的。把与指针搅和在一起的“类型”这个概念分成“指针的类型”和“指针所指向的类型”两个概念，是学习指针的关键点之一。

把指针声明语句中的指针名字和名字左边的指针声明符*去掉，剩下的就是指针所指向的类型。

```cpp
// p指向 int 类型变量。
int* p;

// p指向签名为 void (int) 类型的函数。
void (*p)(int)
```

## 指针的值

指针的值是指针本身存储的数值，是一个4字节（32位机器）或者8字节（64位机器）的整数。它可以被用作内存地址对某块内存进行访问。

我们说一个指针的值是 X，就相当于说该指针指向了以 X 为首地址的一片内存区域。

## 复杂指针类型

有些嵌套多层复杂指针类型不是很容易辨识，认识它们需要一定的练习。

```cpp
// easy. type: int*
int* a;

// p是指针，指向数组，数组元素类型为int，大小为3。
int (*p)[3];

// p是指针，指向数组，数组元素类型为int*，大小为3。
int* (*p)[3];

// p是函数指针，函数参数为int，返回值为int*[3]类型的指针。
int *(*p(int))[3];
```

为了避免复杂类型嵌套带来的麻烦，好的代码应该用typedef层层定义它们，方便代码被别人阅读。

```cpp
// not recommended
int* (*p)[3];
// good
typedef int* INT_POINTER_ARRAY[3];
INT_POINTER_ARRAY* p;

// not recommended
int *(*p(int))[3];
// good
typedef int INT_ARRAY[3];
typedef INT_ARRAY* (*FUNCTION) (int);
FUNCTION p;
```