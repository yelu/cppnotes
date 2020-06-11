# 从源代码到可执行程序

C++代码文件分为两类：

* 头文件(Header)，通常以.h/.hpp作为扩展名。
* 和源文件(Source)。通常以.c/.cpp/.cc作为扩展名。

可执行程序的构建需要经历两个主要步骤：

* 编译。编译器对每个源文件独立进行编译，每一个源文件就是一个编译单元。
* 链接。解决源文件之间符号的相互依赖。

```cpp
// hello_world.cpp

#include <cstdio>

#define SUM(a,b) (a+b)

int sum(int a, int b){
    return a + b;
}

int main(int argc, char* argv[]) {
    int res = sum(1, 2);
    printf("%d", res);
    res = SUM(1, 2);
    return 0;
}
```

## 预处理(Preprocess)

编译源文件之前的一步是调用预处理器(cpp)对源文件进行预处理，生成一个[预编译之后的中间文件](hello_world.i)。具体的操作包括：

* 递归读取`#include`的头文件并替换到源文件中。
* 宏定义替换。

```bash
cpp hello_world.cpp hello_world.ii
```

```cpp
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
    res = (1 +2);
    return 0;
}

```

## 编译(Compile)

接下来，编译器cc1plus将中间文件`hello_world.i`编译成汇编代码[hello_world.s](hello_world.s)。

```bash
/usr/lib/gcc/x86_64-linux-gnu/7/cc1plus hello_world.ii -Og -o hello_world.s
```

## 汇编(Assemble)

```bash
as -o hello_world.o hello_world.s
```

## 链接(Link)

```bash
g++ -v -o hello_world hello_world.cpp
```
