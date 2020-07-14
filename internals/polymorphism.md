# 多态与对象模型

## 单一继承的对象布局

当类中定义了虚函数或者继承了定义虚函数的基类时，类的对象布局中会产生一个虚函数指针，指向该类型的虚函数表(Virutal Functions Table)。有的编译器将虚函数表放在对象头部，有的放在尾部。放在头部的好处是，对象数据成员相对起始位置总是偏移一个指针的大小，比较方便计算。放在尾部的好处是，对象的内存结构保持了和C语言的兼容性。

## 多继承的对象布局

```cpp
class Left
{
public:
    virtual void print();
private:
    int y;
}

class Right
{
public:
    virtual void print();
private:
    int z;
};

class Bottom: public Left, public Right
{
public:
    void print();
private:
    int yz;
};
```

![Polymorphism and Memory Layout](polymorphism.png)

在把派生类指针赋给基类指针时，编译器会安插代码进行偏移量调整，使基类指针指向正确的基类子对象起始位置。例如，当基类指针`Left* left`被派生类指针`new Bottom()`初始化时，`left`指针无需被调整(偏移量为0)，而`right`被初始化时，会被加上一个8字节的偏移量。

根据虚函数表调用虚函数`print()`时，调用的是`Bottom`类型的函数，此时要进行一次this指针调整，调整的值就是虚函数表中的`Top Offset`。如果是通过`left`指针调用的，不需要调整，如果是`right`，则要将`this`减去8字节使得`this`指针指向`Bottom`对象的起始位置。

```cpp
Left* left = new Bottom(); // left += 0
left->print();  // this += 0

Right* right = new Bottom();  // right += 8
right->print();  // this -= 8
```

C++编译器对虚函数的实现多有不同，了解现实中MSVC编译器的实现，可以参考[Reversing Microsoft Visual C++ Part II: Classes, Methods and RTTI](http://www.openrce.org/articles/full_view/23)。

## 练习

**1.** 虚函数表和虚基类表可以共享吗？

对象的虚函数表和虚基类表（如果有虚继承）可以放在一起，取决于编译器的实现。

**2.** 多继承时，类实例中的所有subobject可以共享一个虚函数表吗？

**3.** 同一个类的所有实例可以共享同一个虚函数表吗？多继承时呢？

**4.** 析构函数为什么需要申明为virtual？

**5.** 什么情况下C++的类和C的struct是兼容的？

**6.** 提出一种新的解决虚函数调用的方案。
