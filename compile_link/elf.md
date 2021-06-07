# ELF

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

对于拥有静态或线程存储期的对象（全局和静态变量），如果没有显式初始化，C标准规定执行zero initilization。对于这部分变量，只需要在运行程序时分配好空间，然后填充零就可以了，文件中不需要去保存零值。因此，初始化和未初始化（或初始化为零）的静态/线程存储期对象需要分开存储，前者对应Data段，后者对应BSS段。

## 优化可执行文件大小

gcc的`-flto`开关会在链接时刻分析未被使用的符号，在目标文件中删除它们。对于以下代码，`square_add`是需要导出的函数，`add`被`square_add`调用，理想情况下，LTO优化应该保留`add`和`square_add`，移除`mul`。

```cpp
// File: test_lto.cpp

__attribute__ ((noinline)) int mul(int m, int n);
__attribute__ ((noinline)) int add(int m, int n);
__attribute__((visibility("default"))) int square_add(int m, int n);


int mul(int m, int n) {
    int res = m*n;
    return res;
}

int add(int m, int n) {
    int res = m + n;
    return res;
}

int square_add(int m, int n) {
    int res = add(m*m, n*n);
    return res;
}
```

对于静态库，这个优化不应该起作用。因为静态库的作用就是被其它目标链接，裁剪其中的代码通常不是我们的意图。

对于动态库，由于没有main函数，默认情况下，所有函数、变量和类符号都会被导出。这一行为可以通过显式地指定要导出的符号来改变。对于那些未指定被导出的符号，只要没有被指定导出的符号用到，就可以被链接器删除。对于可执行文件，显式的入口是main函数，只要没有被main函数使用到的符号都会被删除。

```bash
user@ubuntu: g++ -O2 -flto -fvisibility=hidden test_lto.cpp -shared -o libtest_lto.so
user@ubuntu: nm -C libtest_lto.so
0000000000200e90 d _DYNAMIC
0000000000201000 d _GLOBAL_OFFSET_TABLE_
                 w _ITM_deregisterTMCloneTable
                 w _ITM_registerTMCloneTable
00000000000005a0 T square_add(int, int)
0000000000000590 t add(int, int)
0000000000000660 r __FRAME_END__
```

## Exercises

**1.** Why can't I __declspec(dllexport) a function from a static library?

[[link]](https://devblogs.microsoft.com/oldnewthing/20140321-00/?p=1433)[[cache]](why_cant_export_function_from_static_library.html)

## References

* [Linux系统中编译、链接的基石-ELF文件](https://zhuanlan.zhihu.com/p/375510683) [[cache]](ref/Linux_ELF.html)
* [Reduce Unused Code](https://tetzank.github.io/posts/removing-unused-code/)
* [Reducing {fmt} library size 4x using Bloaty McBloatface](https://www.zverovich.net/2020/05/21/reducing-library-size.html)
