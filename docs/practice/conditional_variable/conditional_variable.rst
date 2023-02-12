==============================
Conditional Variable
==============================

Conditonal Variable用于线程间状态的同步。等待线程B等待另一个线程A中发生的操作致使“某个条件成立”，然后唤醒自己往下执行。如果没有条件变量，线程B就需要执行忙等待：不停地检查条件是否成立，直到成立为止，CPU利用率低下。

基于以下原因，条件变量本身不会被单独使用，而是需要与一个 `mutex` 配合使用。这是初次接触条件变量的时候，通常难以理解的地方。

- 在需要条件变量的时候，我们也同时需要在线程A和B之间传递数据。
- lost wakeup
- spurious wakeup

熟悉了条件变量之后，就会发现，条件变量的使用几乎是基于相同的模式。我们只要依据这个模式，就可以很容易地使用条件变量了。producer-consumer模式就是这样一个模式。producer线程生产数据，consumer线程消费数据，当producer线程生产了数据之后，就需要通知consumer线程，让consumer线程消费数据。这个时候，就需要条件变量了。基于线程池的 `ThreadPoolExecutor` 就是遵循这个模式，使用条件变量实现的。

.. code-block:: c++

    #include <condition_variable>
    #include <iostream>
    #include <thread>

    std::mutex mutex;
    std::condition_variable cond_var; 

    bool data_ready{false};

    void waiting_for_work() {
        std::cout << "Waiting " << std::endl;
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock, []{ return data_ready; });   // (4)
        std::cout << "Running " << std::endl;
    }

    void setdata_ready() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            data_ready = true;
        }
        std::cout << "Data prepared" << std::endl;
        cond_var.notify_one();                          // (3)
    }

    int main() {
        std::thread t1(waiting_for_work);               // (1)
        std::thread t2(setdata_ready);                  // (2)
        t1.join();
        t2.join();
    }

Python中条件变量的使用方式也是类似的。加锁和解锁的操作，都是在 `with` 语句中完成的。 `wait_for` 和 `notify` 也是在 `with` 语句中完成的。

.. code-block:: python

    # Consume an item
    with cv:
        cv.wait_for(an_item_is_available)
        get_an_available_item()

    # Produce one item
    with cv:
        make_an_item_available()
        cv.notify()

References
==============================

- `C++ Core Guidelines: Be Aware of the Traps of Condition Variables <https://www.modernescpp.com/index.php/c-core-guidelines-be-aware-of-the-traps-of-condition-variables>`__
- `Conditional Variable in Python <https://docs.python.org/3/library/threading.html#condition-objects>`__