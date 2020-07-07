## Spin Lock

Spin lock是这样一类锁实现，在获取锁的过程中，它在一个循环中不停的检查锁的可用性直到锁可用为止，这一过程称为spin。因为线程在获取锁的过程中执行的是无用的轮询，这一过程被称为是busy-waiting的。

为什么需要spin lock？如果受保护的代码只会执行很短的时间，我们可能会不希望互斥过程中发生上下文切换，因为上下文切换的开销可能大于忙等待的开销。此时spin lock就变得非常有用。这种追求性能的做法多用于操作系统的内部实现中，用户代码较少使用。

有时我们会听到这样的说法：使用spin lock时，等待获取锁的进程将反复检查锁是否释放，不会进入睡眠状态，同时持有自旋锁的进程也不允许睡眠。确切地说，这一说法并不准确，这只是Linux等系统内核中一种通常的实现方式。

跟踪Linux spin_lock(&mr_lock)的实现可以看出，preempt_disable()使得spin lock保护的代码区域工作在非抢占的状态，即使获取不到锁，在获取锁的“自旋”过程中也是禁止抢占的。这就是以上说法产生的源头。

```cpp
#define spin_lock(lock) _spin_lock(lock)
#define _spin_lock(lock) __LOCK(lock)
#define __LOCK(lock) /
do { preempt_disable(); __acquire(lock); (void)(lock); } while (0)
```

究其原因，首先，既然使用spin lock就希望避免上下文切换开销，以上实现显式地直接禁止了抢占以保证效率，无论是在spin过程中还是在锁获取之后，这是合理的做法。但是，如果某种实现在等待过程中不禁止抢占也是合法的，并不妨碍被称为spin lock，只要它在一个循环中不停地尝试获取锁。

其次，如果在获取锁之后调用sleep，在单核CPU上会导致死锁。假设线程A首先获得锁，随后A sleep主动让出CPU，kernel 调度到进程B ，B执行同一段代码也试图去获取同样的锁，由于自旋过程中禁止抢占B会一直占用cpu，kernel 就不可能再调度到A，A 也就不可能去释放锁。结果是A永远sleep下去，B永远spin下去，导致了死锁的发生。因此确切来说，spin lock保护的代码不是不可以调用sleep，而是这样做是不明智的。

## Spin lock&mutex

信号量mutex是sleep-waiting的。当没有获得mutex时，会有上下文切换，将自己加到忙等待队列中，直到另外一个线程释放mutex并唤醒它。CPU不会花费过多计算用于轮询锁的可用性，而是用于调度别的线程执行。

而自旋锁spin lock是busy-waiting。当没有可用的锁时，就一直忙等待并不停的进行锁请求，直到得到这个锁为止。这个过程中cpu始终处于忙状态，不能做别的任务。

自旋锁避免了进程上下文的调度开销，因此只对线程阻塞时间很短的场合才更有效率。

## Implement the spin lock

A spinlock mutex can be implemented in userspace using an atomic_flag

```cpp
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>
 
std::atomic_flag lock = ATOMIC_FLAG_INIT; // must not forget the initialization
 
void f(int n)
{
    for (int cnt = 0; cnt < 100; ++cnt) {
        while (lock.test_and_set(std::memory_order_acquire))  // acquire lock
             ; // spin
        std::cout << "Output from thread " << n << '\n';
        lock.clear(std::memory_order_release);               // release lock
    }
}
 
int main()
{
    std::vector<std::thread> v;
    for (int n = 0; n < 10; ++n) { v.emplace_back(f, n); }
    for (auto& t : v) { t.join(); }
}
```
