# 答面试官

## 整形提升

在一个表达式中，凡是可以使用int或unsigned int类型做右值的地方也都可以使用有符号或无符号的char型、short型和Bit-field。如果原始类型的取值范围都能用int型表示，则其类型被提升为int，如果原始类型的取值范围用int型表示不了，则提升为unsigned int型，这称为Integer Promotion。做Integer Promotion只影响上述几种类型的值，对其它类型无影响。C99规定Integer Promotion适用于以下几种情况：

第一，如果一个函数的形参类型未知，例如使用了Old Style C风格的函数声明，或者函数的参数列表中有...，那么调用函数时要对相应的实参做Integer Promotion，此外，相应的实参如果是float型的也要被提升为double型，这条规则称为Default Argument Promotion。我们知道printf的参数列表中有...，除了第一个形参之外，其它形参的类型都是未知的，比如有这样的代码：

    char ch = 'A';
    printf("%c", ch);
    ch要被提升为int型之后再传给printf。

第二，算术运算中的类型转换。有符号或无符号的char型、short型和Bit-field在做算术运算之前首先要做Integer Promotion，然后才能参与计算。例如：

    unsigned char c1 = 255, c2 = 2;
    int n = c1 + c2;
    计算表达式c1 + c2的过程其实是先把c1和c2提升为int型然后再相加（unsigned char的取值范围是0~255，完全可以用int表示，所以提升为int就可以了，不需要提升为unsigned int），整个表达式的值也是int型，最后的结果是257。假如没有这个提升的过程，c1 + c2就溢出了，溢出会得到什么结果是Undefined，在大多数平台上会把进位截掉，得到的结果应该是1。

除类型提升外，还有其它[很多情况](http://learn.akae.cn/media/ch15s03.html)会造成编译器执行类型的转换。

## 浮点数内存结构

```cpp
float a1=1.0f; 
float b1=0.0f; 
cout<<((int)a1==(int&)a1)<<endl; //false
cout<<((int)b1==(int&)b1)<<endl; //true
```

1.0f的编码跟整数1的编码是不一样的，用（int&）转换的话，会直接把浮点编码转为相应的数值，所以1.0f被转换过来将会变成很大的数。但用(int)转换的话，则是安全的，也就是直接转换为浮点中的整数部分，即1.0f会被转换为1 


## 为什么析构函数中不能抛出异常

禁止异常传递到析构函数外有两个原因。

第一，能够在异常转递的堆栈辗转开解（stack-unwinding）的过程中，防止terminate被调用。

有两种情况下会调用析构函数。第一种是在正常情况下删除一个对象，例如对象超出了作用域或被显式地delete。第二种是异常传递的堆栈辗转开解（stack-unwinding）过程中，由异常处理系统删除一个对象。

在上述两种情况下，调用析构函数时异常可能处于激活状态也可能没有处于激活状态。遗憾的是没有办法在析构函数内部区分出这两种情况。因此在写析构函数时你必须保守地假设有异常被激活，因为如果在一个异常被激活的同时，析构函数也抛出异常，并导致程序控制权转移到析构函数外，C++将调用terminate函数。这个函数的作用正如其名字所表示的：它终止你程序的运行，而且是立即终止，甚至连局部对象都没有被释放。

第二，它能帮助确保析构函数总能完成我们希望它做的所有事情。

如果一个异常被析构函数抛出而没有在函数内部捕获住，那么析构函数就不会完全运行（它会停在抛出异常的那个地方上）。如果析构函数不完全运行，它就无法完成希望它做的所有事情，例如资源没有完全释放。

## 构造函数的调用顺序

首先，调用基类构造函数，调用顺序按照他们的继承时声明的顺序。

其次，调用内嵌成员对象的构造函数，调用顺序按照他们在类中声明的顺序。

最后，派生类的构造函数体中的内容。

构造函数的调用次序完全**不受构造函数初始化列表中变量出现的先后次序影响**，只与基类的声明次序和成员对象在的声明次序有关。

析构函数的析构次序和构造函数相反。

## 引用和指针的区别

引用的声明基本格式：引用类型& 引用名=被引用对象 

**“引用声明时使用的运算符&”跟“取地址运算符&”、“位异或运算符&”没有任何关系。**

在一些编译器中，引用的内部实现和指针并无两样。如果参考其他语言的思想的话，可以得到结论：引用就是指针常量，声明时必须同时初始化，以后不能再引用其它对象。在c++中，引用在语法上与指针有着明显的差异，但是他们并没有本质不同，引用是c++中实现的一种限制比较严格的常量指针，**它在参与任何运算之前自动解引用**。

## const关键字
   
const通常用于以下两种场合：

* 修饰变量。表明变量不能更改，可以替代宏的部分功能，且有类型安全检查。const修饰指针时，如果位于*右侧紧挨着变量，则该指针的指向是不可修改的；如果位于*左侧，则指向的变量的内容是不可修改的。
* 修饰类成员函数。被const修饰的类成员函数不能修改类的成员变量且只能调用其它该类的非const成员函数。如果一个const成员函数需要修改类的某个成员变量则需要使用mutable关键字。

```cpp
// const在前面
const int nValue； //nValue是const
const char *pContent; //*pContent是const, pContent可变
const (char *) pContent;//pContent是const,*pContent可变
char* const pContent; //pContent是const,*pContent可变
const char* const pContent; //pContent和*pContent都是const
// const在后面，与上面的声明对等
int const nValue； // nValue是const
char const * pContent;// *pContent是const, pContent可变
(char *) const pContent;//pContent是const,*pContent可变
char* const pContent;// pContent是const,*pContent可变
char const* const pContent;// pContent和*pContent都是const
```

## Why am I getting an error converting a Foo** → Foo const**?

[The reason](http://www.parashift.com/c++-faq-lite/constptrptr-conversion.html) the conversion from Foo** → Foo const** is dangerous is that it would let you silently and accidentally modify a const Foo object without a cast:

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
    p->modify();         // Ouch: modifies a const Foo!!
    ...
}
```

## static 关键字

static关键字有两层含义：

* 控制变量的存储位置。申明为static的变量存储在静态全局区，而不是堆栈上。这样变量在具有原本作用域的同时也能全局持久化。
* 控制名字的可见性，影响链接方式。它的反义词是extern。被extern修饰的局部变量、全局变量、全局函数都只有文件作用域。

## mutable和volatile关键字

A mutable field can be changed even in an object accessed through a const pointer or reference, or in a const object, so the **compiler knows not to store it in R/O(Read Only) memory**. 

A volatile location is one that can be changed by code the compiler doesn't know about (e.g. some kernel-level driver), so **the compiler knows not to optimize e.g. register assignment of that value under the invalid assumption that the value "cannot possibly have changed"** since it was last loaded in that register. Very different kind of info being given to the compiler to stop very different kinds of invalid optimizations.

关键字mutable是C＋＋中一个不常用的关键字，他只能用于类的非静态和非常量数据成员。如果一个类的成员函数被声明为const类型，表示该函数不会改变对象的状态，也就是该函数不会修改类的非静态数据成员。但是有些时候需要在该类函数中对类的数据成员进行赋值，这个时候就需要用将该数据成员声明为mutable了。

volatile是c/c＋＋中一个鲜为人知的关键字，该关键字告诉编译器不要持有变量的临时拷贝（例如存在寄存器中的拷贝），而是每次都从内存中读取该变量后再使用。它可以适用于基础类型，如：int,char,long......也适用于C的结构和C++的类。当对结构或者类对象使用volatile修饰的时候，结构或者类的所有成员都会被视为volatile。多线程中共享的状态变量通常需要用valatile来修饰防止编译器优化导致一个线程对变量的更新无法被另外的线程及时读取到。

## sizeof的一些牛角尖问题

sizeof(i++)之后，i的值会怎样？答案是不变。记得大一初学C语言时想研究一下sizeof与函数有什么区别，得到的结果只是一些语法上的差别；学了汇编之后看看编译器生成的代码，才发现sizeof在编译时直接给定了一个常值，而非在运行时求值。进而又分析过sizeof(表达式)的结果，清楚了类型提升原理。但我之前没有注意过表达式中出现副作用的问题，于是在sizeof(i++)的问题上犹豫了。现在经过查阅资料和实验，结论是：sizeof在大多数情况下是编译时定值的，表达式中的任何副作用（包括有副作用的运算符、函数调用等）都不会发生。这里说“大多数情况”，排除了针对C99的新特性——不定长数组（variable length array）的特例。参考这篇文章（http://rednaxelafx.javaeye.com/blog/225909），如果sizeof运算符的参数是一个不定长数组，则该需要在运行时计算数组长度。

sizeof('a')的结果是多少？这个要看是在C中还是C++中了。根据C99标准的规定，'a'叫做整型字符常量（integer character constant），为int型，故结果是4（对于32位机器）；而ISO C++规定，'a'叫做字符字面量（character literal），为char型，故结果是1。C强调了'a'的“数”属性，而C++强调了'a'的“字符”属性。

sizeof('ab')的结果又是多少？'ab'这种语法我以前没有注意到。经查，这叫做“多字节字符常量”（multi-character character constant），它限制在单引号中包含2至4个字节。根据标准，多字节字符常量的语义由编译器的实现决定。在我测试的gcc 4.0和VS2008中，如果int a = 'abcd'，则a == 0x61626364。sizeof('ab') == sizeof('abc') == sizeof('abcd') == 4。

那么sizeof(L'a')呢？虽然wchar_t是在源代码级可移植的宽字符，但其大小依赖于操作系统或编译器的定义。无论C或C++，wchar_t字面量本身就是wchar_t类型的常量，所以sizeof(L'a')就等于sizeof(wchar_t)。在我在32位Windows和Linux平台下分别为2和4。

## string的copy on write

一些编译器对string的实现采用了[COW(copy on write)机制](http://www.cnblogs.com/promise6522/archive/2012/03/22/2412686.html)。即string对象相互复制时不会立即申请新的内存拷贝内部存储的字符缓冲区，而是在有可能写入时再申请内存进行拷贝。

copy on write机制有其缺点，导致一些编译器没有支持。

## 全局变量初始化顺序

在c++里面，是以.cpp文件作为编译单元的，当.cpp文件被编译成.o文件后，各.o文件在与库文件进行连接组成了可执行文件。假如这些.o文件和库文件里面有全局变量的话，编译器能为保证的只是，当进入main函数时，所有全局对象已经初始化了。同一编译单元中的全局变量，按照他的定义的顺序进行初始化；但这些不同编译单元中的全局对象，具体哪一个先初始化，在通常情况下是不可控的，因为但 C++ 标准并没有规定不同翻译单位间全局对象的初始化顺序。

按照这个分析，以下的代码可能工作，也可能不工作（cout 是 C++ 用于输出的全局对象，和我们自己的对象位于不同的翻译单位）

```c++
class A {
  A() {
　  cout << "A::A()";
  }
  ~A() {
　  cout << "A::~A()";
  }
};
A a;
```

OK，你会说这段代码绝对运行正确，也就是说 cout 总是比我们的对象先初始化以及后析构。这是有原因的——虽然 C++ 标准并没有明确规定，但各 C++ 编译器都按照类似的方式实现了对全局对象初始化顺序的控制，否则的话，C++ 库就无法按照预期的方式工作了（如果不允许在全局对象构造函数中使用 cout 可能不少程序员会疯掉）。

## 数据对齐

现代计算机中内存空间都是按照byte划分的，从理论上讲似乎对任何类型的变量的访问可以从任何地址开始，但实际情况是在访问特定变量的时候经常在特定的内存地址访问，这就需要各类型数据按照一定的规则在空间上排列，而不是顺序的一个接一个的排放，这就是对齐。

为什么要对齐？各个硬件平台对存储空间的处理上有很大的不同。一些平台对某些特定类型的数据只能从某些特定地址开始存取。其他平台可能没有这种情况，但是最常见的是如果不按照适合其平台要求对数据存放进行对齐，会在存取效率上带来损失。比如有些平台每次读都是从偶地址开始，如果一个int型（假设为32位系统）如果存放在偶地址开始的地方，那么一个读周期就可以读出，而如果存放在奇地址开始的地方，就可能会需要2个读周期，并对两次读出的结果的高低字节进行拼凑才能得到该int数据。显然在读取效率上下降很多。这也是空间和时间的博弈。

自身对齐值：
* 基本数据类型自身的对齐值：char自身对齐值为1，short型为2，int,float为4，double为8，单位字节。
* 结构体或者类的自身对齐值：其成员中自身对齐值最大的那个值。对于内嵌结构体的结构体，对齐值递归计算。

指定对齐值。#progma pack (value)时的指定对齐值value。

有效对齐值是自身对齐值和指定对齐值中小的那个值。有效对齐值N是最终用来决定数据存放起始地址的值**，最重要。有效对齐N，就是表示“对齐在N上”，也就是说`该数据的存放起始地址%N=0`。

当计算一个结构体占用内存大小的时候，要分别考虑一下两点：
* 数据自身的对齐；
* 结构体自身对齐。就是结构体成员变量占用总长度还需要是结构体有效对齐值的整数倍。

## 自定义类型转换

通过重载operator()可以实现自定义类型到其它类型转换，这个其实是通过仿函数的形式提供类似类型转换的语法：

```c++
typename operator()()
```

带有一个参数的构造函数会生成一个类型转换函数，将构造参数的参数类型转换为该自定义类型，explicit关键字会阻止这种转换函数的生成。

```c++
class Woo
{
private:
	double mm;
public:
	/*explicit*/ Woo(double a);
	Woo() {mm=0;};
	void show();
 
	//operator double() const;
};
```

重载隐式转换函数，可以实现自定义类型转为其它类型，并且可以像使用强制类型转换一样使用它们。但要注意以下几点：
* 转换函数必须是类方法。
* 转换函数不能指定返回类型。
* 转换函数不能有参数。

```c++
// typeName是要转换为的类型
operator typeName();
```

## [Additional FAQ](https://isocpp.org/faq)
