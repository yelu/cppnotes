# 内存布局

## Storage Duration

在C++标准中，所有的对象都有自己的存储存续时间（[Storage Duration](https://en.cppreference.com/w/cpp/language/storage_duration)）。截止到C++17，存续时间分为四类。

第一类，automatic。所有未声明为static、extern或thread_local的局部对象都有automatic storage duration，它们在进入一个代码块时被创建，离开代码块时被销毁。

第二类，static。所有在命名空间级别定义的对象或者声明为static/extern的对象（包括局部对象）都有static storage duration，它们在程序启动时被创建，退出前被销毁。

第三类，thread。用thread_local声明的对象具有thread storage duration，它们在线程创建时被创建，结束前被销毁，每个线程拥有自己的独立拷贝。

最后一类，dynamic。用动态内存申请函数（new）创建的对象具有dynamic storage duration，它们的生命周期需要使用者显式地控制，通过delete销毁。

## 进程地址空间

以上对象在一个进程的地址空间中如何摆放，是由操作系统和编译器联合决定的。通常情况下，在[C/C++的内存](https://www.geeksforgeeks.org/memory-layout-of-c-program/)分为以下几个区域:

* 栈。由编译器自动分配释放。
* 堆。一般由程序员分配释放。
* 初始化的数据段。
* 未初始化的数据段。
* 代码段。

![Memory Layout for x86 and C](memoryLayoutC.jpg)

```cpp
int a = 0; // 全局初始化区
char *p1; // 全局未初始化区
void main()
{
    int b; //栈
    char *p3 = "123456"; // 123456{post.content}在常量区，p3在栈上
    static int c = 0; // 全局初始化区
    p1 = (char *)malloc(10); // 分配得来得10字节的区域在堆区
}
```
