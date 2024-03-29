# Coroutine

Coroutine是C++ 20中提供的新特性。促使C++ Coroutine产生的直接需求有两个：
* 异步回调编程
* 迭代器

## 异步

奶茶店人很多，等不及的顾客下完单离开逛商场，店家制作完成后通过小程序通知顾客来取。这就是异步的全部，和代码中的异步在概念上没有区别。

促使异步模式被采用的动机通常是有些操作执行速度太慢，调用方无法接受一直等待下去，不得不放弃等待。异步执行耗时任务的常见模式是，维持一个任务队列(Task Queue)和后台线程池(Worker Thread Pool)。Worker Thread不停地从队列中拿任务执行，执行完成后通过callback或者完成队列通知调用方。通过将任务交给线程池执行，调用方不会再被阻塞在调用处。

```cpp
// notify by callback
// 调用方执行async函数时注册一个回调函数，然后立即返回.异步操作完成后，callback函数会被执行。
async_call(arg1, arg2, ..., callback);

// notify by completion queue
// 调用方post task，然后通过事件循环获取完成的任务
post_task_async(task);
for(;;) {
    std::vector<Task> completed_tasks = get_completed_tasks();
    for(Task& t : completed_tasks) {
        // do something
    }
}
```

![Async Tasks](pic/async_task.png)

常见的耗时任务分为两类：CPU密集型(CPU-bound)和IO密集型(IO-bound)。在对系统资源的有效利用方面，二者有所不同，导致异步的底层实现方式也需要有所区别。对于CPU-bound任务，线程池模型是合适的。而对于IO-bound的任务，将IO操作放进单独的Worker线程，功能上没有问题，但这仅仅是将等待的工作从调用方线程转移到了另一个线程中去，资源利用率方面没有改进。IO-bound的任务是由于磁盘和网络等设备相对CPU过慢，让CPU等待是浪费资源，不如让这些设备在数据就绪时主动通知CPU，CPU在这段时间内可以用于执行其它运算。IO操作作为异步任务的代表，在很多工具库或者文章里面，提到异步甚至指的就是异步IO。为了不打断本节的连贯性，异步IO将在下一节单独介绍。

尽管CPU-bound和IO-bound的任务有所不同，编程语言也还是应该考虑为开发者提供相似的接口和体验。但是这并不是现实，历史上C++对异步的态度一直都是放任的，缺乏统一的标准，导致开发者具体问题具体对待，长期处于分化状态。例如，处理网络IO时，有人喜欢libevent/asio/mudoo等异步网络库，也有人喜欢直接操作BSD APIs和epoll；处理耗时的CPU运算，有人简单启动一个线程处理，也有人用复杂的状态机去记录状态跳转，是否支持中途取消(Cancellation)和超时(Timeout)也各有各的考虑。反倒是C#在异步方面的设计最为干净和考究，它最先在语言层面提供了稳定的async/await/task APIs，开辟了一条道路引领大家思考编程语言应该如何在语法层面简化异步编程。

## 迭代器

```cpp
class RangeGenerator
{
public:
    RangeGenerator(uint32_t n) {max=n;} 
    int get_next() {
        if(i >= max>) {
            throw std::exception("no more data");
        }
        i += 1;
        return i - 1;
    }
private:
    uint32_t max;
    int i = 0;
}
```

迭代器在Python、C#等中都早有实现，精简的迭代语法很受欢迎。

```python
>>> def CreateIterator(l):
...    for i in l:
...        yield i*i
...
>>> iter = CreateIterator([0, 1, 2]) # create a generator
>>> for i in iter:
...     print(i)
0
1
4
```

## 协程

相比简单的串行同步执行模式，异步回调改进了程序的响应模式，提高了系统的处理能力，但代码的复杂度也增加不少。异步回调将一个原本连贯、完整的执行流程切割成多个片段，断断续续地执行。由于涉及到上下文状态的保存和恢复，回调模式很容易进入一个灾难状态，就是所谓的callback hell。

同步阻塞模式就像我们亲自开飞机，根据现场情况，我们边飞边处理。而异步回调机制要求我们设计一个无人机，事先针对现场可能出现的情况制定出一套完善的处理流程(编织出一个回调函数组成的网络)，之后让它自己起飞(启动select/epoll等IO多路复用器)。一系列现场事件在复杂的回调函数关系网中激活一条执行路径，难度显然更大一些。如果再加上严谨的错误处理、超时机制、资源生命周期管理，回调关系网会变得非常复杂。为了避免考虑不周导致错误，像同步模式那样写代码是更理想的选择。

协程的出现使得开发者可以兼顾执行效率和开发效率。在Coroutine进入C++标准之前，[boost::context](http://www.boost.org/doc/libs/1_57_0/libs/context/doc/html/index.html)就对实现协程提供了支持，实现方式是保存多条执行路径的栈和寄存器。随着Coroutine正式进入C++ 20，微软提出了一个无栈实现。

## 进程、线程和协程

进程和线程(参见附录II)是开发者耳熟能详的概念了。进程是在OS上独立并发运行、资源隔离的执行流程。如果进程内部还需要多个并发流程就使用线程。

协程和进程、线程一样，逻辑上看都是流程并发的某种机制。协程所作的事情，功能上也可以由进程或线程实现，为什么还需要协程呢？

```cpp
// 用线程实现迭代器
```

现代操作系统内核中实现的多进程/线程调度方式通常是抢占式(Preemptive)的，操作系统要兼顾效率和公平。当某个进程/线程消耗了较多的CPU时间，即使没有阻塞操作，操作系统也需要剥夺它的时间片，把CPU让给其它进程/线程。而协程采用的是协作式(Cooperative)的调度机制，它将切换的控制权交给了解代码逻辑的开发者：只在有必要切换的时候(例如迭代器产生了新值)才主动将CPU让出。这种调度机制不仅避免了很多不必要的切换，还因为实现相对简单切换一次的开销更小。

## 协程和Erlang

关于协程，还有另外一个值得了解的角度，这个角度就是Erlang语言。Erlang是最早提供协程并获得大规模商业应用的语言，由Joe amstrang设计，已在Ericsson和Nortel的多款主流电信产品中被使用。它始于1981年开始的一个研究项目，该项目的目的是探索更好的编写电信应用软件的方法。早在那个年代，电信类应用已经是一些大型的高并发程序了，常常经过严密的测试，投入运行后还是会有许多错误。Erlang假设这些程序不可避免地会含有错误，进而寻求在软件包含错误的情况下构建可靠系统的方法。这一设计目标和今天编程语言中协程的目标看起来不同，但Erlang探索解决方案的过程，对理解异步回调的问题，以及协程的优势，很有启发性。

论文指出Erlang的三点设计哲学：故障隔离，并发才是正确的方式，解法即为问题。详细的论证参见附录I。异步回调方式在这三个方面都存在问题：

* 多个执行流程在调度器那里相互交织，不利于故障隔离。
* 调度器本质上是一个分时复用的执行器，将逻辑上并发的流程串行实现，实际上是在避免并发。
* 用串行的方式模拟并行，增加了问题和实现之间概念上的隔阂。

Erlang的并发思想在go语言身上重新焕发了生机。Go的设计目标之一就是原生支持高并发，以降低开发高并发服务的困难。Go采用了接近于协程的思路，而不是异步事件回调。它提供的goroutine可以被近乎无限地创建，由语言来负责高效地管理和切换。

## 附I 《面向软件错误构建可靠的分布式系统》

Joe amstrang(1950-2019)在他的博士论文[Making Reliable Distributed Systems in the Presence of Software Errors](../ref/joe_armstrong_making_reliable_distributed_systems_in_the_presence_of_software_errors.pdf)中对Erlang的设计哲学给出了阐释，总结起来，有三个重点。

**第一，故障隔离。**

在构建可容错软件系统的过程中要解决的本质问题就是故障隔离。不同的程序员会编写不同的模块，有的模块正确，有的存在错误。我们不希望有错误的模块对没有错误的模块产生任何不利的影响。

为了提供这种故障隔离机制，我们采用了传统操作系统中进程的概念。进程提供了保护区域，一个进程出错，不会影响到其他进程的运行。不同程序员编写的不同应用程序分别跑在不同的进程中；一个应用程序的错误不会对系统中运行的其他应用程序产生副作用。

这种选择当然满足了初步的要求。然而因为所有进程使用同一片CPU、同一块物理内存，所以当不同进程争抢CPU资源或者使用大量内存的时候，还是可能对系统中的其他进程产生负面影响。进程间的相互冲突程度取决于操作系统的设计特性。

在我们的系统中，进程和并发编程是语言的一部分，而不是由宿主操作系统提供的。

我们这种基于语言的进程比传统的操作系统进程要轻量得多。在我们的语言里，创建一个进程是非常高效的，要比大多数操作系统中进程的创建快几个数量级[12,14]，甚至比大多数语言中线程的创建都快几个数量级。

故障隔离对于编写可容错系统的软件来说，则是本质性的。每一项独立的活动都在一个完全独立的进程中来执行。这些进程没有共享数据，进程之间只通过消息传递的方式进行通信，这就限制了软件错误造成的影响。

一旦进程之间共享有任何公共资源，譬如内存，或指向内存的指针，或互斥体等等，一个进程中的一个软件错误破坏共享资源的可能性就存在。因为消除大型软件系统中的这类软件错误仍然是一个未解的难题，所以我认为构建大型的可靠系统的唯一现实的方法就是把系统分解成许多独立的并行进程，并为监控和重启这些进程提供一些机制。

**第二，并发才是正确的方式。**

在现实世界中，顺序化的（sequential）活动非常罕见。当我们走在大街上的时候，如果只看到一件事情发生的话我们一定会感到不可思议，我们期望碰到许多同时进行的活动。

如果我们不能对同时发生的众多事件所造成的结果进行分析和预测的话，那么我们将会面临巨大的危险，像开车这类的任务我们就不可能完成了。事实上我们是可以做那些需要处理大量并发信息的事情的，这也表明我们本来就是具有很多感知机制的，正是这些机制让我们能够本能地理解并发，而无需有意识地思考。

然而对于计算机编程来说，情况却突然变得相反。把活动安排成一个顺序发生的事件链被视为是一种规范，并认为在某种意义上讲这样更简单，而把程序安排成一组并发活动则是要尽可能避免的，并常常认为会困难一些。

我相信这是由于几乎所有传统的编程语言对真正的并发缺乏有力支持造成的。绝大多数的编程语言本质上都是顺序化的；在这些编程语言中所有的并发性都仅仅由底层操作系统来提供，而不是由编程语言来提供。

**第三，解法即为问题。**

程序的结构要严格保持与问题的结构一致，即每一个真实世界里的活动都严格映射到我们编程语言中的一个并发进程上。如果从问题到程序的映射比例为1:1，我们就说程序与问题是同构（isomorphic）的。

映射比例为1:1这一点非常重要。因为这样可以使得问题和解之间的概念隔阂最小化。如果比例不为1:1，程序就会很快退化而变得难以理解。在使用非面向并发的编程语言来解决并发问题时，这种退化是非常常见的。在非面向并发的编程语言中，为了解决一个问题，通常要由同一个语言级的线程或进程来强制控制多个独立的活动，这就必然导致清晰性的损失，并且会使程序滋生复杂的、难以复现的错误。

为了解决这些问题，Erlang在语言级别上支持Erlang进程，不同于操作系统进程的是，创建Erlang进程的开销非常小，以至于可以近乎无限的创建。

## 附II 进程和线程

Linus的一封邮件[Linux-Kernel Archive: Re: proc fs and shared pids](ref/fs_and_shared_pids.html)对线程和进程做了非常好的解释。不论线程还是进程，**都是一个附带上下文状态的执行单元**。上下文状态包括：

* CPU state (registers etc)
* MMU state (page mappings)
* Permission state (uid, gid)
* Various "communication states" (open files, signal handlers etc)

通常我们认为，线程拥有独立于其它线程的CPU state和栈内存，而其它资源则是进程内所有线程共享的。从内核实现的角度来看，这只是实现的选择之一，而不是唯一选择，只是大多数操作系统选择了这样来实现进程和线程。

现实中，支持kernel-level threads的现代Unix，都实现了如下[经典的进程/线程模型](../ref/a_gentle_introduction_to_multithreading.html)，教材上见到的也都是这种模型：

* 进程是资源的容器。进程的资源包括程序(program text)、数据(data)、文(opened files)等资源。一个进程拥有对这些资源的所有权，OS则提供保护功能，避免不同进程之间的资源冲突。
* 内核调度执行的基本单位是线程、而非进程。每个进程都有自己的运行状态(state)、优先级、寄存器等，是OS调度的基本单位。
* 进程包含一个或多个线程。同一进程下的各个线程共享资源(address space、open files、signal handlers，etc)，但寄存器、栈等不共享。

## 附III Promise和Future

实现异步机制通常需要用到多线程，并在线程间传递数据。Promise和Future是实现数据传递的常见模式之一，C++标准库对此提供了[支持](https://en.cppreference.com/w/cpp/thread/future)。

![Promise and Future](pic/promise_and_future.jfif)

每个`Promise<T>`对象总是关联一个`Future<T>`对象。Promise“承诺”未来某个时候产出一个T类型的数据，通过与之关联的`Future<T>`对象可以在“未来”某个时候获取该数据。Promise和Future之间的关系类似生产者和消费者。

```cpp
#include <iostream>
#include <thread>
#include <future>

using namespace std;

void wait_for_data(const future<int>& f) {
  // blocking until data available
  int data = f.get();  
  cout << "received" << data << endl;
}

void make_data(promise<int>& p) {
    cout << "task completed, data ready" << endl;
    p.set_value(1);
}

int main() {
    promise<int> p;
    future<int> f = p.get_future();
    thread consumer(wait_for_data, f);
    thread producer(make_data, p);
    consumer.join();
    producer.join();

    return 0;
}
```

## 附IV 异步和智能指针

异步通常意味着不止一条执行路径，它们之间共享数据的ownership是很难确定的，谁都不能是owner，于是只能靠share_ptr。因此，异步callback和智能指针这两个东西，看起来不搭边，但是是相辅相成的。

## References

* [风格之争：Coroutine模型 vs 非阻塞 异步IO(callback)](ref/风格之争-Coroutine模型-vs-callback.md)
* [Asynchronous programming overview](ref/Asynchronous-programming-overview.html) [[link]](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/concepts/async/)
* [Asynchronous programming scenarios](ref/Asynchronous-programming-scenarios.html) [[link]](https://docs.microsoft.com/en-us/dotnet/csharp/async)
* [Asynchronous Programming The Task Model](ref/Asynchronous-Programming-The-Task-model.html) [[link]](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/concepts/async/task-asynchronous-programming-model)