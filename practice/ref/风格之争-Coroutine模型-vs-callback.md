# 风格之争：Coroutine模型 vs 非阻塞 异步IO(callback)

我们在设计一个服务器的软件架构的时候，通常会考虑几种架构：多进程，多线程，非阻塞/异步IO(callback) 以及Coroutine模型。

* 多进程
这种模型在linux下面的服务程序广泛采用，比如大名鼎鼎的apache。主进程负责监听和管理连接，而具体的业务处理都会交给子进程来处理。这里有一篇我以前写的文章具体的解释这种架构的实现。

这种架构的最大的好处是隔离性，子进程万一crash并不会影响到父进程。缺点就是对系统的负担过重，想像一下如果有上万的连接，会需要多少进程来处理。所以这种模型比较合适那种不需要太多并发量的服务器程序。另外，进程间的通讯效率也是一个瓶颈之一，大部分会采用share memory等技术来减低通讯开销。

* 多线程
这种模型在windows下面比较常见。它使用一个线程来处理一个client。他的好处是编程简单，最重要的是你会有一个清晰连续顺序的work flow。简单意味着不容易出错。

这种模型的问题就是太多的线程会减低软件的运行效率。

多进程和多线程都有资源耗费比较大的问题，所以在高并发量的服务器端使用并不多。这里我们重点来研究一下两种架构，基于callback和coroutine的架构。

* Callback- 非阻塞/异步IO
这种架构的特点是使用非阻塞的IO，这样服务器就可以持续运转，而不需要等待，可以使用很少的线程，即使只有一个也可以。需要定期的任务可以采取定时器来触发。把这种架构发挥到极致的就是node.js,一个用javascript来写服务器端程序的框架。在node.js中，所有的io都是non-block的，可以设置回调。

举个例子来说明一下。
传统的写法:
<pre>
 var file = open(‘my.txt’);
 var data = file.read(); //block
 sleep(1);
 print(data); //block
</pre>
node.js的写法:
<pre>
 fs.open(‘my.txt’,function(err,data){
    setTimeout(1000,function(){
       console.log(data);
    }
 }); //non-block
</pre>
这种架构的好处是performance会比较好，缺点是编程复杂，把以前连续的流程切成了很多片段。另外也不能充分发挥多核的能力。

* Coroutine-协程
coroutine本质上是一种轻量级的thread，它的开销会比使用thread少很多。多个coroutine可以按照次序在一个thread里面执行，一个coroutine如果处于block状态，可以交出执行权，让其他的coroutine继续执行。使用coroutine可以以清晰的编程模型实现状态机。让我们看看Lua语言的coroutine的例子。
<pre>
> function foo()
 >>   print("foo", 1)
 >>   coroutine.yield()
 >>   print("foo", 2)
 >> end
> co = coroutine.create(foo) -- create a coroutine with foo as the entry
> = coroutine.status(co)
 suspended
 > = coroutine.resume(co) <--第一次resume
 foo     1
 > = coroutine.resume(co) <--第二次resume
 foo     2
 > = coroutine.status(co)
 dead
</pre>
Google go语言也对coroutine使用了语言级别的支持，使用关键字go来启动一个coroutine(从这个关键字可以看出Go语言对coroutine的重视),结合chan(类似于message queue的概念)来实现coroutine的通讯，实现了Go的理念 ”Do not communicate by sharing memory; instead, share memory by communicating.”。一个例子：
<pre>
func ComputeAValue(c chan float64) {
     // Do the computation.
     x := 2.3423 / 534.23423;
     c <- x;
 }
func UseAGoroutine() {
    channel := make(chan float64);
    go ComputeAValue(channel);
    // do something else for a while
    value := <- channel;
    fmt.Printf("Result was: %v", value);
 }
</pre>
coroutine的优点是编程简单，流程清晰。但是内存消耗会比较大，毕竟每个coroutine要保留当前stack的一些内容，以便于恢复执行。

* Callback vs Coroutine
在业务流程实现上，coroutine确实是更理想的实现，基于callback的风格，代码确实不是那么清晰，你可能会写出这样的代码。

//pseudo code
 socket.read(function(data){
 if(data==’1’)
   db.query(data,function(res){
     socket.write(res,function(){
       socket.read(function(data){
    });
   });
 });
 else
    doSomethingelse(...);
 });
当然你可以使用独立的function函数来代替匿名的函数获得更好的可读性。如果使用coroutine就获得比较清晰的模型。
<pre>
 //pseudo code
 coroutine handle(client){
   var data = read(client); //coroutine will yield when read, resume when complete
    if(data==’1’){
    res = db.query(data);
   …
  }
  else{
    doSomething();
  }
 }
</pre>
但是现实世界中,coroutine到目前为止并没有真正流行起来，第一，是因为支持的语言并不是很多，比较新的语言(python/lua/go/erlang)才支持，但是老一些的语言(java/c/c++)并没有语言级别的支持。第二个原因是因为coroutine的使用可能让一些糟糕的代码占用过多的内存，而且比较难于排查。另外在实现一个工作流的构架中，流的暂停和恢复的时机都是未知的，系统的状态并不能放在内存中存放，都必须序列化，所以coroutine本身要提供序列化的机制，才可以实现稳定的系统。我想这些就是coroutine叫好不叫座的原因。

尽管有很多人要求node.js实现coroutine,Node.js的作者Ryan dahl在twitter上说: ”i’m not adding coroutines. stop asking”.至于他的理由，他在google group上提到：
<pre>
Yes, there have been discussions. I think, I won’t be adding
coroutines. I don’t like the added machinery and I feel that if I add
them, then people will actually use them, and then they’ll benchmark
their programs and think that node is slow and/or hogging memory. I’d
rather force people into designing their programs efficiently with
callbacks.
</pre>
我想这是一种风格的选择，优劣并不是绝对的。
