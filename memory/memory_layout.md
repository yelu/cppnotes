# 内存布局

## Storage Duration

在C++标准中，所有的对象都有自己的存储存续时间（[Storage Duration](https://en.cppreference.com/w/cpp/language/storage_duration)）。截止到C++17，存续时间分为四类。

第一类，automatic。所有未声明为static、extern或thread_local的局部对象都有automatic storage duration，它们在进入一个代码块时被创建，离开代码块时被销毁。这些对象位于stack上。

第二类，static。所有在命名空间级别定义的对象或者声明为static/extern的对象（包括局部对象）都有static storage duration，它们在程序启动时被创建，退出前被销毁。

第三类，thread。用thread_local声明的对象具有thread storage duration，它们在线程创建时被创建，结束前被销毁，每个线程拥有自己的独立拷贝。

最后一类，dynamic。用动态内存申请函数（new）创建的对象具有dynamic storage duration，它们的生命周期需要使用者显式地控制，通过delete销毁。这些对象位于heap中。

## 进程地址空间

以上对象在一个进程的地址空间中如何摆放，是由操作系统和编译器联合决定的。通常情况下，在[C/C++的内存](https://www.geeksforgeeks.org/memory-layout-of-c-program/)分为以下几个区域:

* Stack segment。由编译器自动分配释放。
* Heap segment。由程序员分配释放。
* Data segment。初始化的全局变量和静态变量。
* BSS(Block Started by Symbol) segment。未初始化的全局变量和静态变量。
* Text segment。代码。

![Memory Layout for x86 and C](pic/memoryLayoutC.jpg)

```cpp
int a = 0; // Data
char *p1; // BSS
void main()
{
    int b; // Stack
    // string literals may be in Data(OS/compiler dependent)
    char *p3 = "123456"; 
    static int c = 0; // Data
    p1 = (char *)malloc(10); // Heap
}
```

对于拥有静态或线程存储期的对象（全局和静态变量），如果没有显式初始化，C标准规定执行zero initilization。对于这部分变量，只需要在运行程序时分配好空间，然后填充零就可以了，文件中不需要去保存零值。因此，初始化和未初始化（或初始化为零）的静态/线程存储期对象需要分开存储，前者对应Data段，后者对应BSS段。

## ELF

ELF的全称是Executable and Linking Format，表明了ELF所需要支持的两个功能：执行和链接。在 Linux 系统中，一个 ELF 文件主要用来表示3种类型的文件：

* 可执行文件
* 静态链接库(.lib)
* 动态链接库(.so)

ELF文件主要分为5个部分:

* ELF Header
* Section Header Table
* Sections
* Program Header Table
* Segments. 可以认为和Sections等价。

链接器只关心ELF header, Sections以及Section header table这3部分内容。加载器只关心ELF header, Program header table和Segment这 3 部分内容。

strip删除的是Sections中的symtab和strtab等信息。

## References

* [Linux系统中编译、链接的基石-ELF文件](https://zhuanlan.zhihu.com/p/375510683) [[cache]](ref/Linux_ELF.html)