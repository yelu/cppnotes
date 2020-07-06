# 类与对象模型

## 类的功能

类是面向对象的语言都会提供的概念。它的本质是封装了一组数据和施加于它们之上一组操作的集合，这些数据被称为成员变量（Data Fields），操作被称为成员函数（Member Functions）。

```cpp
class Point
{
public:
    // Member Functions
    Point(double x, double y);
    double calc_distance(Point* p);
private:
    // Data Fields
    double x;
    double y;
};
```

类这一抽象的集合概念是对上的，也就是为语言的使用者准备的。它提供了一系列特性来简化开发者的工作，包括但不限于：

* 数据成员
* 函数成员。
* 构造函数。用于初始化类的对象。
* 访问控制符。用于控制类成员对外的可见性。

## 类的底层实现

通常情况下，了解以上类的对外语法特性之后就可以使用C++进行面向对象开发了。但是了解类向下，也就是会被编译器怎样实现对进一步理解C++也很重要。

C++标准对数据布局的要求是同一个access level的members，排列需要满足“较晚出现的member拥有较高的地址”。也就是说，members不是非得连续排列，甚至不同access level的数据也可以交叉排列。而成员函数在对象的内存布局中并不存在，它们只存在于代码段中。

![Class and its Data Fields in Memory(without padding)](class.png)

到达汇编语言之后，类这一概念也是不存在的。实际上，类的成员函数和普通函数没有任何区别，编译器会通过安插this指针、重命名等操作，将它们转化为普通函数，从而去掉类这个概念，类似下面这样：

```cpp
// no class Point anymore!
void Point_Point(void* this, double x, double y);
double Point_calc_distance(void* this, Point* p);
```

对成员函数的调用同样会被编译器改写、替换：

```cpp
// 1. Constructor
// before
Point* p1 = new Point(1.0, 2.0);
Point p2(3.0, 4.0);

// after
void* p1 = malloc(sizeof(Point));
Point_Point(p1, 1.0, 2.0);
void* pp2 = alloca(sizeof(Point));
Point_Point(pp2, 3.0, 4.0);
Point& p2 = *pp2;
```

```cpp
// 2. Member Function
// before
double dis = p1->calc_distance(p2);

// after
double dis = Point_calc_distance(p1, p2);
```

public/private/protected等访问控制符，也完全是由编译器在编译阶段进行检查和约束的。当有代码访问了类的私有数据或函数，编译器在编译时就会报错。因此，访问控制符并不是隐藏功能的一种“加密技术”，而是防止使用者不小心访问到类作者认为不应该被外部使用的内部实现细节，进而避免不必要的错误。从这个角度看，类的开发者和使用者之间是互惠互利的合作关系，而不是加密和破解的对抗关系。

```cpp
// 编译错误。编译器发现访问了申明为private的数据成员x。
Point p2(3.0, 4.0);
double x = p2.x;
```

## 回顾

类是一个方便开发者的面向对象概念，它会被编译器改写替换，变成两部分：
* 由数据成员组成的内存块。
* 去除类的全局函数。

## 练习

**1.** UserInfo类包含一个成员函数`GetPassword()`，它会返回敏感数据，仅限于类的开发者内部使用。该如何防止该接口被使用者调用到？

**2.** 如何导出一个定义在动态链接库中的类供外部使用？你的方式在不同编译器和平台之间兼容性如何？
