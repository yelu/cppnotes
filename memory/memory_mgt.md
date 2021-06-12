# Memory Allocator

## Allocator in STL

STL container的实现采用了Polices模式，内存分配器类型allocator_type是通过模板参数提供的，container在或者自己实例化一个allocator_type类型的对象，或者复制一个用户传入的alloc。

```cpp
template <class InputIterator>
  vector (InputIterator first, InputIterator last,
          const allocator_type& alloc = allocator_type());
```

STL对内存分配器有诸多要求，其中包括以下三点。

第一，无状态。STL要求同一个类型的allocator instances是无差别的，由一个instance1申请的内存可以交由另一个instance2进行释放。因此，在实现自定义内存分配器时，对象内部状态的维护是件难度很高甚至根本不可能完成的事情，除非将状态数据存储在allocator对象外部合适的位置然后在每个对象内部加以引用，对多线程的支持则会带来更大的工作量。C++11在这一点上做出了改进，但并不是所有的STL实现都跟进了标准。

第二，Allocator是container的模板参数，Allocator类型则container类型不同，即使container的数据类型是相同的。这导致container对象间无法相互复制或引用。C++17对此做出了改进。

第三，`Allocator<T>`本身也是模板实现，和容器数据类型绑定。我们无法在`vector<string>`和`vector<int>`间共享同一个allocator实例，也无法做到为每个container对象提供各自不同的allocator。

```cpp
// impossible
Allocator alloc;
vector<string> v1(alloc);
vector<int> v1(alloc);

// impossible
Allocator alloc1;
vector<string> v1(alloc1);

Allocator alloc2;
vector<string> v2(alloc2);
```

## A minial STL compatible memory allocator

```cpp
template<typename T>
class MyAllocator {
  public:
    // type alias
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template<class U>
    struct rebind {
        typedef MyAllocator<U> other;
    };

    MyAllocator() throw() {}
    MyAllocator(const MyAllocator&) throw() {}
    template<class U>
    MyAllocator(const MyAllocator<U>&) throw() {}
    ~MyAllocator() throw() {}

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }

    pointer allocate(size_type n, void* hint = 0) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(pointer p, size_type n) {
        ::operator delete(static_cast<void*>(p));
    }

    size_type max_size() const throw() {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    void construct(pointer p, const T& val) {
        new (static_cast<void*>(p)) T(val);
    }

    void destroy(pointer p) { p->~T(); }
};

template<typename T>
bool operator==(const MyAllocator<T>&, const MyAllocator<T>&) {
    return true;
}

template<typename T>
bool operator!=(const MyAllocator<T>&, const MyAllocator<T>&) {
    return false;
}

template<> class MyAllocator<void> {
  public:
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;
    template<class U>
    struct rebind {
        typedef MyAllocator<U> other;
    };
};
```

## Polymorphic Allocators(C++ 17)

历史上内存分配器的种种问题，是各大公司和机构抛弃STL，纷纷致力于实现自己的私有容器库的重要原因，尤其是产品对性能要求很高的一些公司，其中就包括游戏公司EA([EASTL](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2271.html))，以及bloomberg。

终于，在C++ 17中，ea和bloomberg的提议得到了采纳，polymorphic memory resource(pmr)进入了标准，而eastl就是pmr的原型之一。allocator不再是容器的模板参数，而是以实例方式传入容器的构造参数，且与容器数据类型无关。至此，困扰STL多年的内存分配器问题终于得到了根本性的解决。


```cpp
char buffer[256] = {}; // a small buffer on the stack
std::fill_n(std::begin(buffer), std::size(buffer) - 1, '_');
std::pmr::monotonic_buffer_resource pool{std::data(buffer), std::size(buffer)};
std::pmr::vector<std::pmr::string> vec{ &pool };
```

## 内存碎片

内存碎片指的是，一些内存片段虽然没有被分配出去，但是却无法再被分配出去的现象。碎片问题是伴随着内存的申请和释放过程出现的，很难被完全避免。

内存碎片问题发展到极端，可能会出现一种现象：虽然空闲内存的总量满足申请要求，却找不到任何一个连续的片段满足申请要求，导致内存申请失败。这种极端现象不是由一两次内存申请和释放导致的，而是随时间慢慢积累而来的。正由于此，碎片问题不容易被简单的功能性测试发现。但是，对于长期运行的网络服务来说，问题的暴露是必然的，只是时间问题。为了避免崩溃，这些程序在设计和实现之初就要考虑碎片问题，并进行长时间的压力测试来观察内存的使用情况。对于一些部署后才发现问题的服务，有些公司采用了投机取巧的办法，例如定期重启服务，在最坏的情况到来之前重新开始。

## 常见内存分配器实现

https://github.com/mtrebi/memory-allocators/blob/master/README.md

### Arena Allocator

预先分配一大块内存（arena），然后从这块内存的起始位置，依次线性往后分配内存，构造我们需要的对象。过程中只需要记录已分配内存和未分配内存的分界点。

在这些对象全部使用完之后，我们不需要调用对象的析构函数，可以直接释放整片内存。只要保证以后不再使用这些对象即可。这样能够节省析构函数的调用开销，非常高效。

高并发的无状态网络服务通常都适用采用arena策略的分配器。可以为每个服务线程预先分配一个arena allocator，请求处理过程中直接在该allocator上线性往后分配内存，直到结果发送完成。在接收到下一个请求时，直接跳到同一个allocator的头部重新开始分配内存。这样既不需要任何内存释放操作，也没有任何并发处理开销。

### Stack Buffer

Boost/container/small_vector

### tcmalloc

tcmalloc的全称是Thread Caching Malloc，名字已经表明了它的重要设计：为每个线程单独维护内存的cache，在并发情况下，减少内存管理过程中加锁带来的同步开销。

tcmalloc的设计思路并不是很难理解。它通过线程独占的内存cache来避免线程间的竞争开销；通过CentralCache在线程间开辟通道，平衡和共享它们的内存cache；通过PageHeap在系统和应用间做缓冲，避免频繁和系统交互。

tcmalloc是gpertools的一部分，于2005年开源，随后很多公司的高并发在线服务均开始使用它替代glibc的内存管理函数。这里就出现了一个疑问：既然tcmalloc那么好，为什么glibc中自带的malloc/free不直接采用tcmalloc呢？这个问题可以从另一个角度来理解。第三方内存分配器的策略多是针对一些场景特化的，既然没有一个在任何时候都绝对优势的实现，默认的malloc/free选择不作为，简单直白地在系统和应用间做中介，将内存管理问题留给应用本身或其它第三方库去解决。

![tcmalloc原理](pic/tcmalloc.svg)

## References

* [TCMalloc解密](https://wallenwang.com/2018/11/tcmalloc/) [[cache]](ref/TCMalloc_Wallen_Blog.html)
* [TCMalloc分析 - 如何减少内存碎片](https://zhuanlan.zhihu.com/p/29415507) [[cache]](ref/TCMalloc_memory_fragementation.html)