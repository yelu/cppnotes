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

## References

* [Linux系统中编译、链接的基石-ELF文件](https://zhuanlan.zhihu.com/p/375510683) [[cache]](ref/Linux_ELF.html)
* [Reduce Unused Code](https://tetzank.github.io/posts/removing-unused-code/)
* [Reducing {fmt} library size 4x using Bloaty McBloatface](https://www.zverovich.net/2020/05/21/reducing-library-size.html)
