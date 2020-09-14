# Scope Guard

资源有申请，就要有对应的释放。对C++这种没有自动垃圾回收的语言来讲，保证资源的申请和释放成对出现，避免资源泄露，是开发人员要特别关注的问题。

资源泄漏问题有时是致命的。高并发在线服务，往往一旦启动，持续数月甚至数年都不会停止和重启。随着时间的推移，资源泄露问题会被累积放大，最终导致资源耗尽，系统崩溃。

## 资源管理问题

释放不再使用的资源，看起来是一件很简单的事情，就像下面这样。但是，这种最简单的事，往往藏有一些容易被忽视的细节：在申请和释放之间可能发生异常，导致delete没有被执行。

```cpp
char* ptr = new char[16];
// do something with ptr
delete ptr;
```

这个问题，可以通过恰当地使用`try...catch...`解决。但是，这种方式在C++中算不得优雅。如果资源多了，还会出现很麻烦的嵌套异常处理。

```cpp
char* ptr = nullptr;
try {
    ptr = new char[16];
    // do something with ptr
    delete ptr;
}
catch() {
    if(ptr != nullptr) {
        delete ptr;
    }
}
```

## Scope Guard模式

[RAII](https://en.cppreference.com/w/cpp/language/raii)(resource acquisition is initialization)是在1984–1989年，伴随C++中异常安全的资源管理机制，由Bjarne Stroustrup提出的。RAII的核心思路是把系统资源和对象的生命周期绑定：

* 对象创建时，获取资源（类的构造函数中分配资源）。
* 对象离开作用域或因异常被销毁时，释放资源（析构函数中释放资源）。

Scope Guard模式就是对RAII的有效利用。其核心思想是，将资源和栈上对象的生命周期绑定，在对象析构函数被自动调用时，释放相关联的资源。

常见的系统资源包括内存、打开的文件和共享锁等，对这些资源的释放都已经有约定俗成的模式，好的C++代码应该考虑采用它们。

使用智能指针，消除资源释放代码。

```cpp
// DO NOT
void NoPointerGuard() {
    Object* obj = new Object();
    ...
    delete obj;
}

// DO
void PointerGuard() {
    std::unique_ptr<Object> obj = std::make_unique<Object>();
    ...
}
```

使用lock_guard，避免某些分支下忘记释放锁。

```cpp
// mutex to protect concurrent access (shared across threads)
std::mutex mutex;

void LockGuard () {
    // lock mutex before accessing file
    std::lock_guard<std::mutex> lock(mutex);

    // access shared resource between threads
    ...

    // mutex will be unlocked (from lock_guard destructor) when 
    // leaving scope (regardless of exception)
}
```

绑定文件句柄到栈对象上，使用完毕自动关闭文件。

```cpp
class FileGuard {
public:
    explicit FileGuard(FILE * f):_f(f) {}
    ~FileGuard() { if(_f != NULL) fclose(_f); }

private:
    FILE * _f;
    // noncopyable
    FileGuard(FileGuard const&);
    FileGuard& operator=(FileGuard const&);
};

int main(){
    File* f = fopen("a.txt", "r");
    FileGuard guard(f);
}
```

其实，提供gc的语言，针对非托管的系统资源，通常也有类似以上的设计，例如Python的`with...as...`和C#的`using(...)`。

## 练习

**1.** 用C++11标准加入的智能指针，实现一种能够保证文件句柄被关闭的机制。