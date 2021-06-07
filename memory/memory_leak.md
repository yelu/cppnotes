# 内存泄露

内存泄露问题，通常发生在具有dynamic storage duration的对象上，即用动态内存申请函数（new）在heap上创建了对象，却没有被对应地释放。看起来，只要一次申请对应着一次释放就不会有问题。但现实是，保证这一点相当有难度，Firefox就长期饱受内存泄漏问题折磨。

## Memory Dump(for Windows)

* [effective minidumps](http://www.debuginfo.com/articles/effminidumps.html) [[cache]](ref/effective_minidumps.html)
* [stack and heap dump](http://blog.aaronballman.com/2011/05/generating-a-minidump/) [[cache]](ref/generating_a_minidump.html)
