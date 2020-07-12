# 答面试官

## 整形提升

在表达式中，凡是可以使用int或unsigned int类型做右值的地方也都可以使用有符号或无符号的char、short和bit field。如果原始类型的取值范围都能用int型表示，则其类型被提升为int，如果原始类型的取值范围用int型表示不了，则提升为unsigned int型，这称为`Integer Promotion`。C99规定Integer Promotion适用于以下几种情况。

第一，函数的形参类型未知。例如，使用了Old Style C风格的函数声明，或者函数的参数列表中有`...`，那么调用函数时要对相应的实参做Integer Promotion。此外，实参如果是float型的也要被提升为double型，这条规则称为`Default Argument Promotion`。比如有这样的代码：

```cpp
char ch = 'A';
// printf的参数列表中有...，除了第一个参数，其它参数类型都是未知的
// ch要被提升为int型之后再传给printf
printf("%c", ch);
``` 

第二，算术运算中的类型转换。有符号或无符号的char、short和bit field在做算术运算之前首先要做Integer Promotion，然后才能参与计算。

```cpp
// c1和c2提升会被为int型后再相加。unsigned char的取值范围是0~255，可以用int表示，所以提升为int就可以了，不需要提升为unsigned int。
// 整个表达式的值也是int，最后的结果是257。
// 假如没有提升过程，c1 + c2会发生溢出。
unsigned char c1 = 255, c2 = 2;
int n = c1 + c2;
```

## 浮点数内存结构

1.0f的编码跟整数1是不一样的，用`int&`转换的话，会直接把浮点编码转为相应的数值，所以1.0f被转换过来将会变成很大的数。但用`int`转换的话，则是安全的，也就是直接转换为浮点中的整数部分，即1.0f会被转换为1。

```cpp
float a1 = 1.0f; 
float b1 = 0.0f; 
(int)a1 == (int&)a1; //false
(int)b1 == (int&)b1; //true
```

## 为什么析构函数中不能抛出异常

禁止异常传递到析构函数外有两个原因。

第一，在异常栈展开（Stack Unwinding）过程中，防止terminate被调用。

有两种情况下会调用析构函数。第一种是在正常情况下删除一个对象，例如对象超出了作用域或被显式地delete。第二种是在异常栈展开过程中，由异常处理系统删除一个对象。

调用析构函数时，异常可能处于激活状态也可能没有，没有办法在析构函数内部区分出这两种情况。如果在异常被激活析构函数被调用的同时，再次抛出一个嵌套的异常，会导致程序控制权转移到析构函数外，C++将调用terminate函数。这个函数的作用正如其名字所表示的：它将终止程序的运行。

第二，确保析构函数总能完成我们希望它做的所有事情。

如果异常被析构函数抛出而没有在函数内部被捕获，析构函数会停在抛出异常的地方，没有完成它应该做的所有事情，资源也就没有被完全释放。

## 构造函数的调用顺序

首先，调用基类构造函数，调用顺序按照他们的继承时声明的顺序。

其次，调用内嵌成员对象的构造函数，调用顺序按照他们在类中声明的顺序。

最后，执行派生类构造函数体中的代码。

构造函数的调用次序**不受初始化列表中变量出现的先后次序影响**。析构函数的析构次序和构造函数相反。

## 引用和指针的区别

虽然在语法层面，引用与指针有着明显的差异，在编译器的底层实现中，引用和指针可能并无两样。可以认为，引用就是指针常量，声明时必须同时初始化，以后不能再引用其它变量，在参与任何运算之前自动解引用。

引用声明时使用的运算符`&`，跟取地址运算符、位异或运算符没有任何关系。

## const关键字
   
const通常用于以下两种场合。

第一，修饰变量或指针，表明变量不能更改。可以替代宏的部分功能，且有类型安全检查。

```cpp
// const在前面
const int var; // var不可修改
const char* p_var; // *p_var是const, p_var可变
const (char*) p_var; // p_var是const, *p_var可变
char* const p_var; // p_var是const, *p_var可变
const char* const p_var; // p_var和*p_var都是const

// const在后面
int const var; // var不可修改
char const* p_var; // *p_var是const, p_var可变
(char*) const p_var; // p_var是const, *p_var可变
char* const p_var; // p_var是const, *p_var可变
char const* const p_var;// p_var和*p_var都是const
```

第二，修饰类成员函数。被const修饰的类成员函数不能修改类的成员变量，如果要修改，该成员变量需要使用mutable关键字。

## mutable和volatile关键字

关键字mutable是一个不常用的关键字，只能用于类的非静态和非常量数据成员。如果一个类的成员函数被声明为const类型，表示该函数不会改变对象的状态，也就是该函数不会修改类的非静态数据成员。但是有些时候需要在该类函数中对类的数据成员进行赋值，这个时候就需要用将该数据成员声明为mutable。

volatile是一个更少用的关键字，该关键字告诉编译器不要持有变量的临时拷贝，例如存在寄存器中的拷贝，而是每次都从内存中读取该变量后再使用。它可以适用于基础类型，也适用于C的struct和C++的class。多线程中共享的状态变量通常需要用valatile来修饰防止编译器优化导致一个线程对变量的更新无法被另外的线程及时读取到。

A mutable field can be changed even in an object accessed through a const pointer or reference, or in a const object, so the **compiler knows not to store it in R/O(Read Only) memory**. 

A volatile location is one that can be changed by code the compiler doesn't know about (e.g. some kernel-level driver), so **the compiler knows not to optimize**, e.g. register assignment of that value under the invalid assumption that the value "cannot possibly have changed" since it was last loaded in that register. Very different kind of info being given to the compiler to stop very different kinds of invalid optimizations.

## Why am I getting an error converting a Foo** to Foo const**?

[The reason](http://www.parashift.com/c++-faq-lite/constptrptr-conversion.html) why the conversion from `Foo**` to `Foo const**` is dangerous is that it would let you silently and accidentally modify a const Foo object without a cast.

```cpp
class Foo {
public:
    void modify();  // make some modification to the this object
};

int main() {
    const Foo x;
    Foo* p;
    Foo const** q = &p;  // q now points to p; this is (fortunately!) an error
    *q = &x;             // p now points to x
    p->modify();         // modify a const Foo!!
}
```

## static关键字

static关键字有两个作用。

第一，指定链接方式为内部链接，名字仅在本编译单元可见。它的反义词是extern。

第二，改变变量的存储位置。static变量存储在静态全局区，而不是堆栈上，变量在具有原本作用域的同时也是全局持久化的。

## sizeof的一些牛角尖问题

`sizeof(i++)`之后，i的值会怎样？答案是不变。大多数编译器对sizeof会在编译时求值，而非在运行时求值，表达式中的任何副作用都不会发生。一个例外是，如果sizeof运算符的参数是C99的新特性不定长数组（variable length array），则会在运行时计算数组长度。

`sizeof('a')`的结果是多少？这个要看是在C中还是C++中了。根据C99标准的规定，'a'叫做整型字符常量（integer character constant），类型为int，故结果是4。而ISO C++规定，'a'叫做字符字面量（character literal），类型为char，故结果是1。

`sizeof('ab')`的结果是多少？'ab'这种语法叫做“多字节字符常量”（multi-character character constant），它限制在单引号中包含2至4个字节。根据标准，多字节字符常量的语义由编译器的实现决定。在gcc 4.0中，有以下结果。

```cpp
int a = 'abcd'; // a = 0x61626364
sizeof('ab') == 4
sizeof('abc') == 4
sizeof('abcd') == 4;
```

那么`sizeof(L'a')`呢？虽然wchar_t是可移植的宽字符类型，但其大小依赖于操作系统和编译器的定义。wchar_t字面量就是wchar_t类型的常量，所以sizeof(L'a')就等于sizeof(wchar_t)。在Windows和Linux平台上分别为2和4。

## string的copy on write

一些编译器对string的实现采用了[COW(copy on write)机制](http://www.cnblogs.com/promise6522/archive/2012/03/22/2412686.html)。即string对象相互复制时不会立即申请新的内存拷贝内部存储的字符缓冲区，而是在有可能写入时再申请内存进行拷贝。

copy on write机制有其缺点，一些编译器没有支持。

## 全局变量初始化顺序

C++是以.cpp文件作为编译单元的，当.cpp文件被编译成.o文件后，再进行链接组成可执行文件。假如这些.o文件里面有全局变量的话，编译器能保证的是，当进入main函数时，所有全局对象已经初始化了。

同一个编译单元中的全局变量，按照定义顺序进行初始化。但不同编译单元中的全局对象，具体哪一个先初始化，在通常情况下是不可控的，标准并没有规定不同编译单位中全局对象的初始化顺序。以下的代码可能工作，也可能不工作。

```c++
// cout是标准库中的全局对象，和自定义对象位于不同的编译单元。

class A {
    A() { cout << "A::A()"; }
    ~A() { cout << "A::~A()"; }
};

A a;

int main() {
    return 0;
}
```

但是，这段代码一般都能正确运行，也就是说cout总是比自定义对象先初始化以及后析构。原因是，虽然C++标准没有明确规定，但编译器都按照类似的方式实现了对全局对象初始化顺序的控制，尽可能避免问题。

## 数据对齐

现代计算机的内存是以字节为粒度的，从理论上讲对数据的访问可以从任何地址开始。但实际情况是，内存访问经常是从特定的内存地址开始的。因此，各类型的数据不是顺序的一个接一个的排放，这就是对齐。

为什么要对齐？一些平台对特定类型数据的存取只能从特定的地址开始。另一些平台的情况是，如果不进行合适的对齐，会在存取效率上带来损失。比如，有些平台每次读都是从偶地址开始的，一个int型变量如果存放在偶地址开始的地方，那么一个读周期就可以读出，而如果存放在奇地址开始的地方，就可能会需要2个读周期。本质上，这是空间和时间的博弈。

自身对齐值：

* 基本数据类型自身的对齐值：char自身对齐值为1，short型为2，int,float为4，double为8。
* 结构体或者类的自身对齐值：其成员中自身对齐值最大的那个值。对于内嵌结构体的结构体，对齐值递归计算。

指定对齐值。`#progma pack (value)`时的指定对齐值value。

有效对齐值是自身对齐值和指定对齐值中小的那个值。有效对齐值N是最终用来决定数据存放起始地址的值。有效对齐N，就是表示“对齐在N上”，也就是说`该数据的存放起始地址%N=0`。

当计算一个结构体占用内存大小的时候，要考虑两点：
* 数据自身的对齐；
* 结构体自身对齐。就是结构体成员变量占用总长度还需要是结构体有效对齐值的整数倍。

## 自定义类型转换

通过重载操作符`operator()`可以实现仿函数(Callable, Functor)，看起来很像一个类型转换，但本质上是函数调用，而不是类型转换。

```c++
class Square {
public:
    double operator()(double x) const {
        return x*x;
    }
};
 
int main(void) {
    Square square;
    double res = square(2.0);  // 4
    return 0;
}
```

带有一个参数的构造函数会生成一个隐式的类型转换函数，将构造参数的参数类型转换为该自定义类型，`explicit`关键字会阻止这种转换函数的生成。

```cpp
class Foo
{
public:
	  /*explicit*/ Foo(double a) { };
	  Foo() { };
 
	  //operator double() const;
};
```

重载隐式类型转换函数，可以实现自定义类型转为其它类型，并且可以像使用强制类型转换一样使用它们。转换函数必须是类方法，不能指定返回类型，也不能有参数。

```cpp
class Foo
{
public:
    Foo() { };

    // 将Foo类型转换为double类型
    operator double() const;
};
```

## enable_shared_from_this

## crt runtime linkage

## [duff's device](http://en.wikipedia.org/wiki/Duff's_device)

## [Additional FAQ](https://isocpp.org/faq)
