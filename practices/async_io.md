# 异步IO

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