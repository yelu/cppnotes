# Coroutine



促使异步模式被采用的动机通常有两个：
* IO设备太慢
* 更优雅的代码

## 异步

奶茶店人很多，等不及的顾客下完单离开逛商场，奶茶制作完成后店家通过小程序通知顾客来取。这就是异步的全部，和代码中的异步在概念上没有丝毫区别。

异步执行模式比顺序执行的同步模式要复杂，促使异步模式被采用的动机通常是有些操作速度太慢以至于不得不放弃忙等待。调用方启动一个慢速任务然后去做其他的，任务结束后通知调用方任务完成，调用方拿到结果继续执行后续操作。本质上，异步是一个带有上下文状态的执行流程，被断断续续地执行。

耗时的慢速任务分为两类：CPU密集型(CPU Bound)和IO密集型(IO Bound)。从使用方的角度看，二者的表现是类似的，都是执行时间长。但是从对系统资源的合理利用角度看，二者存在很大的差异。CPU-bound的任务无论我们如何调整执行模式，仍然需要CPU去执行，区别只是调用方要不要被阻塞在调用之处。而IO-bound的任务是由于磁盘和网络等设备相对CPU过慢，让CPU等待是浪费资源，不如让这些设备在数据就绪时主动通知CPU，而CPU在这段时间内可以用于执行其它运算。

异步执行CPU-bound的任务需要启动后台线程，并提供一个回调就基本完成了。

```cpp

```

而高效地异步执行IO-bound的任务要复杂得多。像处理CPU-bound任务那样，将IO操作放进单独的子线程去处理，将等待的职责从调用方线程转移到了另一个线程中去，功能上是没有问题的，但是在资源利用率方面没有改进。在用户量很大的高并发场景下，这种模式有很大改进空间。但是改进的基础却必须涉及整个系统栈，包括硬盘/网卡、驱动、操作系统和应用层代码，因为任务完成这个“信号”最终是由硬盘/网卡等硬件发出的，它经过的每个环节都要能高效地处理信号，才能达到最终“整体高效”的效果。在很多工具库或者文章里面，提到异步甚至指的就是异步IO，为了不打断本节的连贯性，关于它的技术演进将在下一节单独介绍。

尽管从实现上讲，异步执行CPU-bound和IO-bound任务由很多不同。但从使用者角度讲，两种类型的任务应该拥有类似的接口来提供统一的使用体验。但是这并不是现实，C++对异步的态度历史上一直都是放任的，缺乏统一的标准，导致开发者具体问题具体对待，长期出于分化状态。例如，处理网络IO时，有人喜欢libevent，asio等基于callback的封装实现，也有人喜爱直接操作BSD API和epoll；处理耗时的CPU运算，有人简单启动一个线程处理，也有人用复杂的状态机库去记录状态跳转，支不支持中途取消运算(Cancelation)和超时(Timeout)也各有各的考虑。反倒是C#在异步方面的设计最为干净和考究，它最先在语言层面提供了稳定的async/await/task APIs，开辟了一条道路引领大家思考编程语言应该如何在语法层面简化异步编程。

现代操作系统都为异步IO提供了特定的API，Linux上是select/epoll等，Windows上是完成端口(IO)



## 迭代器

## 协程

## POSIX AIO and Kernel AIO

* [Linux AIO](https://github.com/littledan/linux-aio)

On linux, the two AIO implementations are [fundamentally different](https://stackoverflow.com/questions/8768083/difference-between-posix-aio-and-libaio-on-linux).

The POSIX AIO is a user-level implementation that performs normal blocking I/O in multiple threads, hence giving the illusion that the I/Os are asynchronous. The main reason to do this is that:

* it works with any filesystem
* it works (essentially) on any operating system (keep in mind that gnu's libc is portable)
* it works on files with buffering enabled (i.e. no O_DIRECT flag set)

The main drawback is that your queue depth (i.e. the number of outstanding operations you can have in practice) is limited by the number of threads you choose to have, which also means that a slow operation on one disk may block an operation going to a different disk. It also affects which I/Os (or how many) is seen by the kernel and the disk scheduler as well.

The kernel AIO (i.e. io_submit() et.al.) is kernel support for asynchronous I/O operations, where the io requests are actually queued up in the kernel, sorted by whatever disk scheduler you have, presumably some of them are forwarded (in somewhat optimal order one would hope) to the actual disk as asynchronous operations (using TCQ or NCQ). The main restriction with this approach is that not all filesystems work that well or at all with async I/O (and may fall back to blocking semantics), files have to be opened with O_DIRECT which comes with a whole lot of other restrictions on the I/O requests. If you fail to open your files with O_DIRECT, it may still "work", as in you get the right data back, but it probably isn't done asynchronously, but is falling back to blocking semantics.

Also keep in mind that io_submit() can actually block on the disk under certain circumstances.