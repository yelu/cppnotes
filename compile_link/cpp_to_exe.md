# 从源代码到可执行程序

很多人的C++之旅或许是从书上的Hello World程序开始的，通过点击Visual Studio或者CLion的一些按钮很快就能成功打印出“hello world”。在背后，IDEs是通过调用具体的编译器命令行(msvc、g++等)完成工作的。

```cpp
#include <iostream>
using namespace std;

int main() {
    cout << "hello world" << endl;
    return 0;
}
```

g++一行命令就能输出可执行程序。更细致地看的话，过程涉及四个处理步骤：预处理、 编译、汇编和链接。前三步中，编译器是对每个源文件(.cpp/.cc)独立进行的，每一个源文件就是一个处理单元。最后一步，链接器会统筹所有目标文件，解决跨源文件的符号依赖。

```bash
# gnu toolchain
g++ -c hello_world.cpp

# visual studio toolchain
cl /EHsc hello_world.cpp
```

![Compile and Link](pic/compile_link.png)

关于理解这一过程的重要性，《Computer System: A Programmer's Perspective》在开头“计算机系统漫游”阶段就指出来了，随后又在“链接”一章详细解释了程序链接和运行的各个细节。

> 对于像hello这样简单的程序，我们可以依靠编译系统生成正确有效的机器代码。但是，有一些重要的原因促使程序员必须知道编译系统是如何工作的。
> * 优化程序性能。
> * 理解链接时出现的错误。
> * 避免安全漏洞。

## 预处理(Preprocess)

编译源文件之前的一步是调用预处理器(cpp)对源文件进行预处理，生成一个[预编译之后的中间文件](hello_world.i)。预处理这一步可以被看作是对源代码文件所作的一次“字符串替换”。具体的操作包括以下三类。

* 递归读取`#include`头文件并替换到源文件中。
* 宏定义替换。
* 条件编译指令。


头文件在被include“拷贝”进源文件之后，就没有任何存在的必要了。也就是说，头文件没有任何编译意义。

```cpp
// hello_world.cpp

#include <stdio.h>

#define SUM(a,b) (a+b)

int sum(int a, int b) {
    return a + b;
}

int main(int argc, char* argv[]) {
    int res = sum(1, 2);
    printf("%d", res);
    res = SUM(1, 2);
    return 0;
}
```


```bash
cpp hello_world.cpp hello_world.ii
# or
g++ -E hello_world.cpp > hello_world.ii
```

```cpp
// hello_world.ii

# 1 "hello_world.cpp"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4

...

namespace std
{
  typedef long unsigned int size_t;
  typedef long int ptrdiff_t;
  typedef decltype(nullptr) nullptr_t;

}

...

# 3 "hello_world.cpp"
int sum(int a, int b){
    return a + b;
}

int main(int argc, char* argv[]) {
    int res = sum(1, 2);
    printf("%d", res);
    res = (1 +2); // macro definition is repalced
    return 0;
}

```

## 编译(Compile)

接下来，编译器cc1plus将中间文件`hello_world.ii`编译成汇编代码[hello_world.s](code/hello_world.s)。这一步是真正的C++词法、语法分析发生的地方。

```bash
/usr/lib/gcc/x86_64-linux-gnu/7/cc1plus hello_world.ii -Og -o hello_world.s
# or
g++ -S hello_world.cpp
```

## 汇编(Assemble)

```bash
as -o hello_world.o hello_world.s
# or
g++ -c hello_world.cpp
```

## 链接(Link)

链接器需要解决没有定义在`hello_world.o`中的外部符号。例如，`printf`就是定义在预先编译好的C兼容库中，一般随着编译器一起被安装，需要通过命令行参数`-lc`引入。缺少这个链接库，便会导致一个链接错误。

```bash
/usr/lib/gcc/x86_64-linux-gnu/7/collect2 \
    -dynamic-linker /lib64/ld-linux-x86-64.so.2 \
    -o hello_world \
    /usr/lib/gcc/x86_64-linux-gnu/7/../../../x86_64-linux-gnu/Scrt1.o \
    /usr/lib/gcc/x86_64-linux-gnu/7/../../../x86_64-linux-gnu/crti.o \
    /usr/lib/gcc/x86_64-linux-gnu/7/crtbeginS.o \
    -L/usr/lib/gcc/x86_64-linux-gnu/7 \
    -L/usr/lib/gcc/x86_64-linux-gnu/7/../../../x86_64-linux-gnu \
    -L/usr/lib/gcc/x86_64-linux-gnu/7/../../../../lib \
    -L/lib/x86_64-linux-gnu \
    -L/lib/../lib \
    -L/usr/lib/x86_64-linux-gnu \
    -L/usr/lib/../lib -L/usr/lib/gcc/x86_64-linux-gnu/7/../../.. \
    hello_world.o \
    -lstdc++ -lm -lgcc_s -lgcc -lc -lgcc_s -lgcc \
    /usr/lib/gcc/x86_64-linux-gnu/7/crtendS.o \
    /usr/lib/gcc/x86_64-linux-gnu/7/../../../x86_64-linux-gnu/crtn.o
# or
g++ -v -o hello_world hello_world.o
```

```bash
# remove -lc will result in a linking error. 
/usr/bin/ld: hello_world.o: undefined reference to symbol 'printf@@GLIBC_2.2.5'
```

## 练习

**1.** 如何导出一个定义在动态链接库中的类供外部使用？你的方式在不同编译器和平台之间兼容性如何？

**2.** 阅读 Chapter 1 and Chapter 7 of book "Computer Systems A Programmer’s Perspective".

## References

* [The inside story on shared libraries and dynamic loading](ref/the_inside_story_on_shared_libraries_and_dynamic_loading.pdf)
* [Why can't I __declspec(dllexport) a function from a static library](https://devblogs.microsoft.com/oldnewthing/20140321-00/?p=1433) [[cache]](ref/why_cant_export_function_from_static_library.html)
