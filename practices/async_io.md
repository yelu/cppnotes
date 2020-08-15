# 异步IO

由于磁盘、网卡等输入输出设备和CPU的处理速度不匹配，异步IO是程序处理输入输出的常用模式。CPU发起IO请求之后并不等待执行完成，而是继续其它操作，设备在IO操作完成之后通知发起线程IO请求完成。

常见的IO操作有两类：磁盘IO和网络IO。基于Linux一切都是文件的哲学，多数处理IO的系统调用都能兼顾磁盘文件和网络设备读写。不过，存取磁盘数据和处理网络请求毕竟是不同的，内核(尤其是Linux)对它们的抽象封装还在改进当中，相应的编程模式还是有所区别。

## Socket网络编程

在计算机网络中，使用最为广泛的通信协议是TCP/IP协议簇。进程间通信采用的是客户端/服务器模型，运行在客户端和服务器端主机应用层的程序通过建立点对点的连接来实现通信，连接的端点称为[socket](http://en.wikipedia.org/wiki/Network_socket)，它由网络层的IP地址和传输层的TCP(UDP)端口组成。一个连接由通信两端的socket对唯一确定，这对socket称为套接字对，由一个二元组表示。服务器和客户端通过对各自socket的读写实现数据的收发。

```
(ClientAddr:ClientPort, ServerAddr:ServerPort)
```
    
![Hardware and Software Organization of Internet Application](./HardwareAndSoftwareOrganizationOfInternetApplication.png)

操作socket的一组最有名的API是[Berkeley Socket APIs](http://en.wikipedia.org/wiki/Berkeley_sockets#Socket_API_functions)。它最早于1983年在BSD Unix 4.2上发布。然而，由于AT&T的专利保护着UNIX，到1989年Berkeley大学才能够自由地发布它们。其设计简单、实用，后来逐渐成为了网络socket操作的事实标准。包含的主要函数有：

* socket()
* bind()/listen()/accept(). Used on the server side.
* connect(). Used on the client side.
* send()/recv(), or write()/read(), or sendto()/recvfrom()
* close()
* select(), poll()

## 阻塞IO

基于BSD Socket APIs，client和server端的工作流程如下，`read`和`write`调用只有在数据到达或者出现错误时才会返回。在服务器端，操作流程为：

1. 建立监听socket listen_fd。
2. 通过bind将listen_fd绑定到端口上。
3. 开始在listen_fd上进行监听。
4. 一旦有client连接，listen操作就会返回。此时调用accept操作返回一个新的socket conn_fd。
5. 通过对conn_fd的read和write操作实现和client的通信。

![TCP Workflow](./TCPWorkflow.png)

```cpp
// create socket
int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
if(listen_fd < 0) { return 1; }
struct sockaddr_in svr_addr;
svr_addr.sin_family = AF_INET;
svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
svr_addr.sin_port = htons (8888);

// bind
if(bind(listen_fd, (sockaddr*)&svr_addr, sizeof(svr_addr)) < 0){ return 1; }

// listen
if(listen(listen_fd, 3 /*backlog*/) < 0) { return 1; }

struct sockaddr_in client_addr;
socklen_t client_addr_len = sizeof(client_addr);

// accept
int conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
if(conn_fd < 0) { continue; }

// read and send data
int data_recv = 0;
if(read(conn_fd, &data_recv, sizeof(data_recv)) != sizeof(data_recv)) { continue; }
write(conn_fd, &data_recv, sizeof(data_recv));
close(conn_fd);
```

采用阻塞IO的server在处理并发连接时存在困难。主线程在接受一个连接后，会阻塞在read处等待client发送数据，如果此时client迟迟不发送数据或者网络状况不佳，主线程就会在read处一直等待，无法处理新的连接请求。

解决该问题可以借助多线程/多进程。主线程在accept之后创建一个线程执行read和write操作，然后马上返回accept处，等待新的连接请求。这个解决方案可以处理并发连接，但是线程作为操作系统的资源，创建和销毁的是有开销的，不可能不停地创建。

使用线程池复用线程能不能解决问题呢？我们创建一个包含若干个线程的线程池，主线程作为生产者，负责接收连接请求，并将其放入队列中。线程池作为消费者，从队列中取出连接进行处理。这依然不是个合格的方案，假设线程池有20个线程，某client端同时发起20个socket连接，之后拒绝发送数据，所有线程都会在read处阻塞，这就形成了一个简单的DDoS攻击。其实，等待read/write的开销并没有因为使用线程池而减少，只是转嫁给了处理线程。想要提高并发处理能力，要解决的根本问题是**避免CPU花时间等待IO就绪**。做到这点，需要IO多路复用(I/O multiplexing)。

## I/O Multiplexing

POSIX最早提供的阻塞IO多路复用调用是[select](http://linux.die.net/man/2/select)。它可以帮助应用程序同时监听多个socket上的可读、可写事件，当有socket可读可写时返回这些socket。阻塞IO模型中等待IO就绪的工作被交给了select函数，由内核态代码高效实现。

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

select多路复用能够处理比同步IO多得多的并发量，在一些要求没那么高的场景下已经足够应付了。但缺点也还是有：

* 监听socket数目受限。fd_set是一个bit数组，这个数组的大小是固定的，跟内核编译时代码中一个宏有关，其默认值为1024。这导致select能够同时监听的socket数目是受限的。

* 低效地轮询。这点从select函数的参数中就能看出端倪。[select](http://linux.die.net/man/2/select)函数的第一个参数是需要监听的最大的socket id，每次select调用系统采用轮询的方式逐个检查这些socket中有无就绪。如果没有这个最大id，内核就要遍历整个socket id的取值范围。该参数缓解了问题，但没有彻底解决问题，一旦有某个socket id很大，这种优化就失效了。另外，每次select返回时就绪的socket很可能只占监视集合的很小一部分，遍历所作的无用功其实很多。

2002年，linux 2.5.44版本内核包含了另一个多路复用API：[epoll](http://man7.org/linux/man-pages/man7/epoll.7.html)，它的出现解决了select的问题。epoll的出色性能使它很长一段时间内都是linux上实现高并发服务的首选方案。

* epoll所支持的fd上限是最大可以打开文件句柄数。
* 内核在epoll的实现上，取消了内部等待进程对socket就绪事件的轮询，采用设备就绪时回调的方式将就绪的socket加入就绪列表。



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

不同平台对这种IO多路复用思路都提供了实现，它们的工作模式是类似的：

1. 在一个事件循环中，用户在注册感兴趣的fd和事件。对应30-32行刚收到连接时，设置监听错误事件。
2. 多路复用API（select/epoll等）利用内核机制负责监听事件。对应第9行的epoll_wait。
3. 事件发生后返回，并通知用户发生事件的fd及发生了何种事件。对应10-38行，检查epoll返回的就绪fd和事件，进行对应的业务处理。
4. 回到1，重新开始事件循环。对应上面代码的无限for循环。

| Pltform       | API           |
| ------------- |-------------  |
| Linux         | epoll/select  |
| Free BSD      | kqueue        |
| Windows       | IOCP          |

## 使用库：libevent和Asio

直接使用底层的select/epoll等IO多路复用系统调用，尽管效率很高，但开发复杂度比较高，也不好跨平台。通过封装IO多路复用机制，很容易开发出基于回调的异步操作库。遵循这种思路实现的跨平台封装有：

* libevent。轻量级，C风格。
* boost::asio。较libevent重量，C++面向对象风格。

libevent的设计风格和POSIX APIs一脉相承，比较薄的一层封装，接口不多不少，深得开发人员喜爱，Redis和MemeryCached的网络操作都是基于它的。

尽管接口形式不同，这些库的设计思路是类似的，都是基于事件回调。下面是一个用Boost::Asio实现的[echo server](https://www.boost.org/doc/libs/1_74_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp)精简后的代码。io_service是一个经过封装的IO多路复用器。借助asio，client-server交互的流程是：

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

目前为止，我们提高并发能力的思路是这样的：

* 同步阻塞。无法并发。
* 多线程。实现简单，效率低。
* 事件循环+回调。高并发，实现复杂。

使用协程(Coroutine)可以兼顾开发效率和执行效率。boost::asio已经完全支持协程，它提供co_spawn函数来启动一个协程，每个异步函数都有协程重载版本，使用起来就像在写同步程序一样。基于协程在C++ 20的落地，asio会在接下来的时间里继续改进对协程的支持，以更加合理、优雅、高效的方式准备加入到C++ 23。

## Socket网络编程值得注意的细节

### TIME_WAIT

### 信号屏蔽

### 句柄资源回收

## POSIX AIO and Kernel AIO

AIO试图把IO操作以统一的异步接口提供开发者，select/epoll等多路复用APIs在AIO中是不存在的，开发者将fd传入AIO API，内核在fd上有数据时直接通知调用方。Windows上的IOCP是一个真正意义上高效的AIO实现，而Linux对[AIO](https://github.com/littledan/linux-aio)的支持较晚，开发持续了很长时间也没有成熟，陷入了泥藻。

目前Linux上常见的AIO应用场景是读写磁盘文件，在网络socket上的稳定应用十分罕见。这导致AIO甚至被理解为异步磁盘IO的特化方案。

On linux, the two AIO implementations are [fundamentally different](https://stackoverflow.com/questions/8768083/difference-between-posix-aio-and-libaio-on-linux).

The POSIX AIO is a user-level implementation that performs normal blocking I/O in multiple threads, hence giving the illusion that the I/Os are asynchronous. The main reason to do this is that:

* it works with any filesystem
* it works (essentially) on any operating system (keep in mind that gnu's libc is portable)
* it works on files with buffering enabled (i.e. no O_DIRECT flag set)

The main drawback is that your queue depth (i.e. the number of outstanding operations you can have in practice) is limited by the number of threads you choose to have, which also means that a slow operation on one disk may block an operation going to a different disk. It also affects which I/Os (or how many) is seen by the kernel and the disk scheduler as well.

The kernel AIO (i.e. io_submit() et.al.) is kernel support for asynchronous I/O operations, where the io requests are actually queued up in the kernel, sorted by whatever disk scheduler you have, presumably some of them are forwarded (in somewhat optimal order one would hope) to the actual disk as asynchronous operations (using TCQ or NCQ). The main restriction with this approach is that not all filesystems work that well or at all with async I/O (and may fall back to blocking semantics), files have to be opened with O_DIRECT which comes with a whole lot of other restrictions on the I/O requests. If you fail to open your files with O_DIRECT, it may still "work", as in you get the right data back, but it probably isn't done asynchronously, but is falling back to blocking semantics.

Also keep in mind that io_submit() can actually block on the disk under certain circumstances.

## 练习

**1.** socket和端口的区别和联系。

**2.** server在accept之后会返回一个新的socket，这个socket对应的server端口是什么？对这个socket的读写如何区分不同的client端？
