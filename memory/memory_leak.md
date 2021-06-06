# 内存泄露

## Storage Duration

在C++标准中，所有的对象都有自己的存储存续时间（[Storage Duration](https://en.cppreference.com/w/cpp/language/storage_duration)）。截止到C++17，存续时间分为四类。

第一类，automatic。所有未声明为static、extern或thread_local的局部对象都有automatic storage duration，它们在进入一个代码块时被创建，离开代码块时被销毁。这些对象位于stack上。

第二类，static。所有在命名空间级别定义的对象或者声明为static/extern的对象（包括局部对象）都有static storage duration，它们在程序启动时被创建，退出前被销毁。

第三类，thread。用thread_local声明的对象具有thread storage duration，它们在线程创建时被创建，结束前被销毁，每个线程拥有自己的独立拷贝。

最后一类，dynamic。用动态内存申请函数（new）创建的对象具有dynamic storage duration，它们的生命周期需要使用者显式地控制，通过delete销毁。这些对象位于heap中。

内存泄露问题，通常发生在最后一类，即在heap上动态申请的对象没有被对应地释放。

## Memory Dump(for Windows)

* [effective minidumps](http://www.debuginfo.com/articles/effminidumps.html) [[cache]](ref/effective_minidumps.html)
* [stack and heap dump](http://blog.aaronballman.com/2011/05/generating-a-minidump/) [[cache]](ref/generating_a_minidump.html)
