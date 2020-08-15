# 异步IO

由于磁盘、网卡等输入输出设备和CPU的处理速度不匹配，异步IO是程序处理输入输出的常用模式。CPU发起IO请求之后并不等待执行完成，而是继续其它操作，设备在IO操作完成之后通知发起线程IO请求完成。

## 网络IO模型

在计算机网络中，使用最为广泛的网络通信协议是TCP/IP协议簇，服务进程间通信采用的是客户端-服务器模型。运行在客户端和服务器端主机应用层的程序通过建立点对点的连接来实现通信，连接的端点称为[网络socket](http://en.wikipedia.org/wiki/Network_socket)，它由网络层的IP地址和传输层的TCP(UDP)端口组成。一个连接由通信两端的socket唯一确定，这对socket称为套接字对，由一个二元组表示：

```
(ClientAddr:ClientPort, ServerAddr:ServerPort)
```
    
![Hardware and Software Organization of Internet Application](./HardwareAndSoftwareOrganizationOfInternetApplication.png)

操作网络socket的一组最有名的API是[Berkeley Socket APIs](http://en.wikipedia.org/wiki/Berkeley_sockets#Socket_API_functions)。它最早于1983年在BSD Unix 4.2上发布。然而，由于AT&T的专利保护着UNIX，到1989年Berkeley大学才能够自由地发布它们。其设计简单、实用，后来逐渐成为了网络socket操作的事实标准。包含的主要函数有：

* socket()
* bind()/listen()/accept(). Used on the server side.
* connect(). Used on the client side.
* send()/recv(), or write()/read(), or sendto()/recvfrom()
* close()
* select(), poll()

## Synchronous & Blocking IO

基于BSD socket API，我们首先从一个最简单的echo server的实现开始，逐步来看每一个方案的问题在哪里，是哪些因素推进了我们选择新的解决方案，哪些技术为目前工业界主流的高并发方案所采用。

![TCP Workflow](./TCPWorkflow.png)

在服务器端，操作流程如下：

1. 建立监听Socket listen_fd。
2. 通过bind将listen_fd绑定到端口上。
3. 开始在listen_fd上进行监听。
4. 一旦有client连接，listen操作就会返回。此时调用accept操作返回一个新的socket conn_fd。
5. 通过对conn_fd的read和write操作实现和client的通信。

```C++
int main(int argc, char **argv)
{
    int listenFd, connFd;
    pid_t childpid;
    socklen_t clientAddrLen;
    struct sockaddr_in clientAddr, svrAddr;
    int srvListenPort = 9877;
    
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0) { return 1; }
    bzero(&svrAddr, sizeof(svrAddr));
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svrAddr.sin_port = htons (srvListenPort);
    if(bind(listenFd, (sockaddr*)&svrAddr, sizeof(svrAddr)) < 0){ return 1;}
    if(listen(listenFd, LISTENQ) < 0) {return 1;}
    char buff[1024];
    for ( ; ; ) 
    {
        clientAddrLen = sizeof(clientAddr);
        connFd = accept(listenFd, (SA *) &clientAddr, &clientAddrLen);
        if(connFd < 0) {continue;}
        // read and send data.
        int dataRecv = 0;
        if(read(connFd, &dataRecv, sizeof(dataRecv)) != sizeof(dataRecv)) {continue;}
        write(connFd, &dataRecv, sizeof(dataRecv));
        close(connFd);
    }
    close(listenFd);
    return 0;
}
```

这种IO操作方式称为同步阻塞IO(synchronous & blocking IO):

* 处理流程是同步的。bind、listen、accept、read和write返回即代表它们需要负责的工作已经执行完毕，这和之后提到的异步回调方式形成对应。
* IO操作是阻塞的。read和write操作会等待数据到达，直到有数据到达或者出现错误才会返回。

同步阻塞IO模型在处理并发连接时存在困难。在主线程接受一个连接后，会阻塞在read处等待client发送数据，如果此时client迟迟不发送数据或者网络状况不佳，主线程就会在read处一直等待，无法处理新的连接请求。

解决该问题可以借助多线程/多进程。对于上面的echo server，主线程在accept之后新建一个线程，然后马上返回accept处，处理新的连接请求，read和write操作在新的线程中执行。

这个解决方案可以处理并发连接，但是线程作为操作系统的资源，不可能无限制创建，加上线程切换的开销，该方式无法支撑超高并发的连接请求。

使用线程池复用线程能不能解决线程无限制增长的问题呢？我们创建一个包含若干个线程的线程池，主线程作为生产者，负责不断地新建socket连接，并将socket放入一个队列中。另外一个独立线程作为消费者，不断地查看队列中有没有新链接，以及线程池中有没有空闲线程，如果都有，就从线程池中取出空闲线程处理新连接。这样，是不是同时解决了主线程阻塞和线程数目不受控的问题呢？确实解决了，但没有解决本质问题。假设线程池有20个线程，某client端同时发起20个socket连接，之后拒绝发送数据，所有线程都会在read处阻塞，这就形成了一个简单的DDoS攻击。

如果换一个角度来看的话，其实，等待read和write的时间并没有因为使用多线程而减少，只是平摊到了任务处理线程之中。因此，如果想要提高并发处理能力，要解决的根本问题是**减少代码等待IO就绪的时间**。要做到这点，我们需要另一种IO模型-同步IO多路复用（synchronous I/O multiplexing）。

## Synchronous I/O multiplexing

POSIX最早提供的同步IO多路复用的接口函数是[select](http://linux.die.net/man/2/select)。它可以帮助应用程序**同时监听**多个socket上的可读、可写事件，当有socket可读可写时返回这些socket。阻塞IO模型中等待IO就绪的时间被交给了select函数，由内核态代码高效实现。一个典型的select事件循环如下，清晰起见，我们略过一些错误处理逻辑。

```cpp
std::set<int> connFds;
int maxFd = -1;
int listenFd = -1；
int maxFd = listenFd;
fd_set readFdSet, allFdSet;
FD_ZERO(&allFdSet);
// 1. add listening socket to socket set to be monitored. 
FD_SET(listenFd, &allFdSet);
for ( ; ; ) 
{
    // 2. Add all socket we want to read data from to read fd set.       
    maxFd = -1;
    std::set<int>::iterator ite = connFds.begain();
    for (; ite != connFds.end(); ite++) 
    {
        FD_SET(*ite, &readFdSet);
        if(*ite > maxFd) {maxFd = *ite;}
    }
    
    // 3. Let select to monitor all sockets in readFdSet.
    select(maxFd + 1, &readFdSet, NULL, NULL, NULL);

    // 4. After select returns. If listenFd still left in readSocketSet, new connection found.
    if (FD_ISSET(listenfd, &rset)) 
    {
        // 3. accept new connection. The accept will never block.
        clilen = sizeof(cliaddr);
        int connFd = accept(listenFd, (SA *)&cliaddr, &clilen);
        connFds.insert(connFd);

        FD_SET(connFd, &allFdSet);	/* add new descriptor to set */
    }

    // 5. Test all connection sockets to see if they are in readFdSet. If it is in, data is ready to read.
    std::set<int>::iterator ite = connFds.begain();
    for (; ite != connFds.end(); ite++) 
    {
        if(FD_ISSET(*ite, &readFdSet)) 
        {
            // 6. Read some bytes. The read will never block.
            int dataRecv = 0;
            read(*ite, &dataRecv, sizeof(dataRecv);
            ...
        }
    }
}
```

select多路复用比同步阻塞IO能够处理多得多的并发量，在一些要求没那么高的场景下已经足够应付了。但缺点也还是有：

* 监听socket数目受限。fd_set是一个bit数组，这个数组的大小是在一个固定值，跟内核编译时代码中一个宏的设置有关，其默认值为1024。这导致select能够同时监听的socket数目是受限的。

* 低效地轮询。这点从select函数的参数中就能看出端倪。[select](http://linux.die.net/man/2/select)函数的第一个参数是需要监听的最大的socket id。每次select调用系统采用轮询的方式一个个检查这些socket中有无socket可读，如果没有这个最大id，系统每次必须遍历整个socket id的取值范围，耗费大量cpu时间。该参数缓解了问题，但没有彻底解决问题，一旦有某个socket号很大，这种优化就失效了。加上每次select返回时可读/可写的socket很可能只占所有监视集合的很小一部分，遍历所作的无用功其实很多。

2002年，linux 2.5.44版本内核包含了另一个多路复用API - [epoll](http://man7.org/linux/man-pages/man7/epoll.7.html)，它的出现解决了select的所有问题。

* epoll所支持的fd上限是最大可以打开文件句柄数。
* 内核在epoll的实现上，取消了内部等待进程对socket就绪事件的轮询，采用设备就绪时回调的方式将就绪的socket加入就绪列表。

epoll的出色性能使它成为linux上实现高并发服务的首选方案。一个精简的epoll事件循环如下：

```cpp
_epollfd = ::epoll_create(10);

// Monitor read event on listen socket.
struct epoll_event event;
event.data.fd = _listen_fd;   
event.events = EPOLLIN;
::epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listen_fd, &event)

for (;;)
{
    int nfds = ::epoll_wait(_epollfd, _events, MAX_EVENTS, -1);
    for (int i = 0; i < nfds; i++)
    {
        int fd = _events[i].data.fd;
        if ((_events[i].events & EPOLLERR) || (_events[i].events & EPOLLHUP))
        {
            // An error has occured on this fd, or the socket is not ready for reading.
            close(fd);
            continue;
        }
        else if (_listen_fd == fd)
        {
            // We have a notification on the listening socket, which means one or more incoming connections. */
            while (true)
            {
                struct sockaddr in_addr;
                char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
                socklen_t in_len = sizeof(in_addr);
                int infd = ::accept(_listen_fd, &in_addr, &in_len);
                struct epoll_event event;
                event.data.fd = infd;
                // only monitor err initially.
                event.events = EPOLLERR | EPOLLHUP;
                epoll_ctl(_epollfd, EPOLL_CTL_ADD, infd, &event);
            }
        }
        else
        {
            // A read or write event happens, read or write some data.
        }
    }
}
```

不同的平台对这种IO多路复用思路都提供了实现：

| Pltform       | API           |
| ------------- |-------------  |
| Linux         | epoll/select   |
| Free BSD      | kqueue         |
| Windows       | IOCP(完成端口)  |

无论是平台和实现，IO多路复用机制的工作模式类似：

1. 在一个事件循环中，用户在注册感兴趣的fd和事件。对应30-32行刚收到连接时，设置监听错误事件。
2. 多路复用API（select/epoll等）利用内核机制负责监听事件。对应第9行的epoll_wait。
3. 事件发生后返回，并通知用户发生事件的fd及发生了何种事件。对应10-38行，检查epoll返回的就绪fd和事件，进行对应的业务处理。
4. 回到1，重新开始事件循环。对应上面代码的无限for循环。

## 融合业务代码

到目前为止，我们并没有考虑业务代码，例如，连接后需要执行CPU密集型的计算。如果业务处理代码都是内存操作，很快就能结束，在事件循环所在的线程中运行就可以了。Redis就采用了这种方式。

如果业务处理代码涉及CPU密集型的操作，将业务代码直接放在事件循环线程中运行，就会阻碍事件循环，影响接收新连接。这时，就是需要增加线程池用于处理CPU密集型业务逻辑的时候了。这是MemoryCached使用的方式。

1. 事件循环线程将就绪的fd放入就绪fd队列readyFdQueue。一个fd对应一个业务连接conn，为了记住conn的业务处理进度，需要为每个conn建立一个状态机connState，记录conn的状态。
2. 监视线程不断检查readyFdQueue和线程池threadPool，如果有就绪fd和空闲线程，就拿出一个线程thread处理该fd。
3. thread根据fd获取该连接的connState结构，根据进度决定接下来需要做什么操作。如果碰到需要IO请求，将fd投入epoll的请求队列requestQueue，将当前状态回写connState。之后，线程退出处理逻辑，等待epoll下一次IO就绪时唤醒该连接继续处理。

## 使用库：libevent和Asio

直接使用底层的API操作，尽管效率很高，但开发难度较大，也不好跨平台。幸运的是，有大量的库对网络IO操作进行了跨平台的封装，其中最著名的有：

* libevent。轻量级，C风格。
* boost::asio。较libevent重量，C++面向对象风格。

libevent的设计风格和POSIX APIs一脉相承，比较薄的一层封装，接口不多不少，深得很多开发人员的喜爱，Redis和MemeryCached的网络操作都是基于它的。

除去一些接口形式上的不同，这些库的设计思路其实类似：都是基于事件回调。下面是一个用Boost::Asio实现的[echo server](http://www.boost.org/doc/libs/1_47_0/doc/html/boost_asio/example/echo/async_tcp_echo_server.cpp)精简后的代码。io_service是一个经过封装的IO多路复用器。借助asio，client-server交互的流程是：

1. server对象构造函数调用start_accept，向acceptor注册完成事件回调server::handle_accept。
2. accept完成后，handle_accept将会被调用，执行session::start函数，同时再次调用start_accept用于监听新的连接。
3. session::start中，async_read_some向io_service注册数据读取完毕的回调函数session::handle_read。
4. 数据读完成后，session::handle_read被调用，其中调用async_write，向io_service注册数据写完毕回调函数session::handle_write。
5. 数据写完成后，session::handle_write被调用，其中再次调用async_read_some，向io_service注册数据读完毕回调函数session::handle_read，继续读取数据。

```cpp
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>

using namespace boost::asio;
using boost::system::error_code;
using ip::tcp

void accept_handler(boost::shared_ptr<tcp::socket> psocket, error_code ec)
{
    if(ec) return;

    start();

    boost::shared_ptr<std::string> pstr(new std::string("hello async world!"));
    psocket->async_read_some(buffer(*pstr),
     boost::bind(&read_done_handler, this, pstr, _1, _2)
     );
}

void read_done_handler(boost::shared_ptr<tcp::socket> psocket, error_code ec)
{
    if(ec) return;

    boost::shared_ptr<std::string> pstr(new std::string("hello async world!"));
    psocket->async_write_some(buffer(*pstr),
     boost::bind(&write_done_handler, this, pstr, _1, _2)
     );
}

void write_done_handler(boost::shared_ptr<std::string> pstr, error_code ec, size_t bytes_transferred)
 {
    if(ec) { std::cout<< "send failed!" << std::endl; }
    else {std::cout<< *pstr << "sent." << std::endl; }
 }

int main(int argc, char* argv[])
{
    io_service &iosev;
    ip::tcp::acceptor acceptor;
    
    boost::shared_ptr<tcp::socket> psocket(new tcp::socket(iosev));
    acceptor.async_accept(*psocket, boost::bind(&accept_handler, this, psocket, _1) );
    
    iosev.run();

    return 0;
}
```

## 使用协程

到目前为止，我们提高并发能力的思路是这样的：

* 同步阻塞。无法并发。
* 多线程。实现简单，效率低。
* 事件循环+回调。高并发，实现复杂。

使用协程(Coroutine)可以兼顾开发效率和执行效率。boost::asio已经完全支持协程，它提供co_spawn函数来启动一个协程，每个异步函数都有协程重载版本，使用起来就像在写同步程序一样。基于协程在C++ 20的落地，asio会在接下来的时间里继续改进对协程的支持，以更加合理、优雅、高效的方式准备加入到C++ 23。

## Socket网络编程值得注意的细节

### TIME_WAIT

### 信号屏蔽

### 句柄资源回收

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

关于协程，还有很多值得思考的，我们把它放在附录二中。
