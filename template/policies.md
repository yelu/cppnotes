# Policies

Polices通常以类模板的模板参数形式提供。它定义了一些函数接口，并向模板类注入实现，即定义了**Function Interfaces**。注入方式或将Policy作为基类继承，或将一个Policy实例化作为成员属性。

通过提供多个Plocies模板类型，还可以将多种不相关的Plocies分开定义和实现。例如，unordered_map就通过继承提供了Hasher和Alloc两个Polices，一个用于提供Hash实现，另一个用于管理内存分配。

```cpp
template<
    class Key,
    class T,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>,
    class Allocator = std::allocator< std::pair<const Key, T> >
> class unordered_map;
```

使用Plocies而不是Virtual Functions的原因，和使用模板还是虚函数来实现多类型系统的理由一致，一个发生在编译时，一个发生在运行时。

### Traitsh和Policies的交互

Data和Function的交互是一个类的核心所在。因此，Traits和Polices是相互密切联系的。

模板库通过剥离对Type和Policies接口的实现，可以极大地统一模板类本身的代码逻辑。
