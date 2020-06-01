# 编程范式

编程范式（Programming Paradigm）指的是指导程序构建的某种“思想”，它直接影响了代码的结构和风格。每一门编程语言的设计都遵循了一种或者多种编程范式，无论它有没有在文档中明确说明。

到今天，编程范式已经发展成为一个[复杂的体系](https://en.wikipedia.org/wiki/Programming_paradigm)。好在日常工作中并不是每一种出现的频率都一样高，本节只打算讨论三种最常见到的范式：

* 过程式，Procedural Programming
* 面向对象，Object-oriented Programming(OOP)
* 函数式，Functional Programming(FP)

这些编程范式每一个在今天看起来都很常见很普通，开发者每天都会遇到它们。所以了解它们就不应该停留在只了解它们代表的内容或者含义本身，更重要的是认识到自己的位置和局限性：

* 程序设计的背后有它的理论体系，不是没有章法的。
* 常见的模式只是众多模式中的一小部分。除了它们，历史上还有很多可供借鉴的模式客观存在。

## 过程式

不过分深究的话，过程（Procedural）可以被认为等同于函数（Function）。用过程式思想写出的代码由一个个函数组成，程序的执行过程就是函数间的相互调用。

典型的支持过程式范式的语言就包括C语言。

```c
typedef struct
{
    float x;
    float y;
} Point;

float calc_distance(Point p1, Point p2)
{
    float delta_x = p1.x - p2.x;
    float delta_y = p1.y - p2.y;
    return sqrt(delta_x * delta_x + delta_y * delta_y);
}

int main()
{
    Point p1 = { .x = 0.0, .y = 1.0 };
    Point p2 = { .x = 2.0, .y = 3.0 };
    float distance = calc_distance(p1, p2);
    printf("%f", distance);
    return 0;
}
```

## 面向对象

面向对象编程（OOP）试图用object来组织和抽象程序的所有功能。object封装了数据（data）和操作数据的函数（functions）供外部调用。

很多现代编程语言都支持OOP，应用非常普遍。其中C++支持OOP但不是仅支持OOP，而Java/.Net等语言从最顶层看就仅支持OOP一种编程范式，即使是作为程序入口的main函数也必须包在一个类中。

```c#
public class Point
{
    public Point(double x, double y)
    {
        this.x = x;
        this.y = y;
    }
    public double CalcDistance(Point p)
    {
        double delta_x = this.x - p.x;
        double delta_y = this.y - p.y;
        return Math.Sqrt(delta_x * delta_x + delta_y * delta_y);
    }
    private double x;
    private double y;
}
                    
public class Program
{
    public static void Main()
    {
        Point p1 = new Point(0.0, 1.0);
        Point p2 = new Point(2.0, 3.0);
        double distance = p1.CalcDistance(p2);
        Console.WriteLine($"{distance}");
    }
}
```

## 函数式


## 忘掉编程范式的机械划分

机械地认识和记忆每种编程范式的特点，并在实践中套用，不应该成为终点。

首先，编程范式和语言不是一一对应的，C++就支持以上三种编程范式。这其中有历史原因，例如完全兼容C，也有与时俱进的考虑，例如加入了lambda表达式等函数式元素。

其次，多元化的支持带来了选择性，也带来了挑战。函数式通常是作为一种局部存在，可以简化代码、提升模块的封装复用性，一般不会喧宾夺主成为顶层设计。

```c
```


而过程式和面向对象就不一样了。混合两种设计会使得代码看起来混乱，变得难以维护，因为它们都会深入顶层，导致明显的风格上的冲突。选择一种比混用两种要更明智一些。这个问题在同时支持过程式和面向对象的语言，如C++中是值得注意。Java和.Net等完全面向对象的语言则直接封死了道路：不支持过程式。

面向对象和过程式这么水火不容吗？答案是否定的。暂时忘掉他们思考程序的本质是什么？是围绕一组数据（data）进行的一系列操作（code），仅此而已。不论是面向对象的语言还是过程式的语言，并不能阻止以下设计的发生：

第一种，语言支持OOP，但并不能阻止数据和操作关联混乱。这时，代码看起来是一个个类，却丧失了逻辑和封装。这是不合理使用面向对象的例子。

第二种，对于没有提供面向对象机制的语言，如C语言，依然可以遵循“数据+操作”的思路产出结构良好的代码。

```c
typedef struct
{
   double x;
   double y;
} Point;

Point* point_create(double x, double y)
{
    Point* p = malloc(sizeof(Point));
    p->x = x;
    p->y = p;
    return p;
}

void point_destroy(Point* p)
{
    free(p);
}

double point_calc_distance(Point* p1, Point* p2)
{
    double delta_x = p1->x - p2->x;
    double delta_y = p1->y - p2->y;
    return sqrt(delta_x * delta_x + delta_y * delta_y);
}
```

因此，虽然面向对象的语言有三大原则之一的“封装(Encapsulation)”特性，也只不过是在语法层面提供了一些便利。最终的实践靠的是开发者心中有无章法，而不是某种理论上的划分。
