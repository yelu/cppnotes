# Memory Allocator

## Allocator in STL

STL container使用Polices模式，提供一个模板类型供自定义内存分配器类型Allocator。然后在内部构造该类型的对象allocator。

```cpp
template <class InputIterator>
  vector (InputIterator first, InputIterator last,
          const allocator_type& alloc = allocator_type());
```

STL对内存分配器有各种要求，以下是其中三点。

第一，无状态。STL要求同一个类型的allocator instances是无差别的，由一个allocator instance申请的内存可以交由另一个instance进行释放。因此，instance内部难以维护状态。如果要实现自定义内存分配器，需要将状态信息存储在合适的位置，如果再考虑多线程难度挺高的。C++11在这一点上做出了改进，但并不是所有的STL实现都跟进了标准。

第二，Allocator是container的模板参数，不同类型的Allocator会导致不同的container对象类型。因此container对象间无法相互复制或引用。C++17对此做出了改进。

第三，`Allocator<T>`本身也是模板实现，和容器类型绑定。我们不能在`vector<string>`和`vector<int>`间共享同一个allocator实例，也无法做到为每个容器类实例提供不同的分配器实例。

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

这些要求有时会带来不便，很多公司和机构致力于设计自己的容器类型，出发点之一就是实现适合自己的内存分配器，例如[EASTL](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2271.html)。

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

## 常见内存分配器实现

https://github.com/mtrebi/memory-allocators/blob/master/README.md

## Arena Allocator

## Stack Buffer

Boost/container/small_vector
