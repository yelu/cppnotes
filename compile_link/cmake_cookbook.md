# CMake简介

在Linux的世界里，从源码编译、安装工具包是件很常见的事情。这个过程中，有个[“三部曲”模式](https://thoughtbot.com/blog/the-magic-behind-configure-make-make-install)经常被采用：

```bash
./configure --prefix=/usr/local/bin/
make
make install
```

按照惯例，`configure`脚本负责查找系统本地的编译工具链，配置一些可定制的参数，例如指定安装目录，最终产出定制化的Makefile文件，供接下来的`make`和`make install`使用。

Kitware的开发人员将这一模式“发扬光大”，做了诸多增强和改进，在2000年前后发布了CMake这个跨平台的编译系统生成工具(Build System Generator)。Kitware目前仍负责维护CMake，代码本身采用社区开源模式管理，免费使用，Kitware提供商业的培训和技术支持。

相比IT产业技术更迭的速度，CMake已经算很古老的项目了。随之可以想象的是，它给人一种历史带来的晦涩感。但是，一路走来，在C/C++编译方面，CMake不是尽善尽美，却也还是做的最好的那个。喜欢还是不喜欢，它都已经成为一种事实标准，不仅绝大多数新的C++项目采用CMake编译，很多老项目也在逐步迁移到它上面来。

## 编译工具栈

编译C/C++，说到底，最终还是靠编译器程序外加一长串命令行配置项和开关完成的。根据平台和喜好，可选的编译工具链(Toolchain)有很多，例如：
* GNU gcc/g++
* [Microsoft MSVC](https://docs.microsoft.com/en-us/cpp/build/reference/compiling-a-c-cpp-program?view=vs-2019), Windows only
* [Clang](https://clang.llvm.org/get_started.html)

```bash
g++ main.cpp -I /path/to/include -L /path/tp/lib -l libbencoding -o foo.exe -std=c++11
```

但是，编译工作通常不是一行命令就能直接搞定的。前前后后难免有一些准备、清理工作和胶水逻辑，这时就需要一个好的构建系统(Build System)来管理整个构建流程。常用的构建系统有：
* GNU Make
* Ninja
* MSBuild
* Apple Xcode

有了构建系统，编译的流程就变成了提供对应系统的配置文件，例如Makefile(GNU Make)或者vcproj(MSBuild)文件，再调用编译系统提供的命令，如`make clean; make`等间接完成编译。

```makefile
PROG=prog
CC=g++
CPPFLAGS=–I/home/tom/prog/include
LDFLAGS=-g -std=c++11 -Wall
OBJS=main.o
$(PROG) : $(OBJS)
    $(CC) $(LDFLAGS) -o $(PROG) $(OBJS)
main.o :
    $(CC) $(CPPFLAGS) -c main.cpp
clean:
    rm -f $(PROG) $(OBJS)
```

CMake这时候的出现提供了又一层更高的抽象，它能以统一的语法生成不同构建工具需要的配置文件，因此，CMake被称为**Build System Generator**。这是个准确且重要的名字：它不是编译工具，也不是构建系统，而是一个**生成构建系统所需流程的工具**。CMake目前支持几乎市面上所有无论是基于命令行还是IDE的构建系统。

![Stack of Build Tools](cmake_stack.png)
