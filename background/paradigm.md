# 编程范式

编程范式（Programming Paradigm）是指导程序构建的某种“思想”或者“方法论”，它从根本上影响了代码的结构和风格。

到今天，编程范式已经发展出一个[很多元的体系](https://en.wikipedia.org/wiki/Programming_paradigm)。熟知所有语言和范式既不太可能，也不见得需要。在现实中，不是每一种范式被应用的频率都一样高，本节只讨论三种最常用的范式：

* 过程式，Procedural Programming
* 面向对象，Object-oriented Programming(OOP)
* 函数式，Functional Programming(FP)

## 过程式

不过分深究的话，过程（Procedural）可以被认为等同于函数（Function）。用过程式思想写出的代码由一个个函数组成，程序的执行过程就是函数间的相互调用。

典型的支持过程式范式的语言就包括C语言。

```c
typedef struct
{
    double x;
    double y;
} Point;

double calc_distance(Point p1, Point p2)
{
    double delta_x = p1.x - p2.x;
    double delta_y = p1.y - p2.y;
    return sqrt(delta_x * delta_x + delta_y * delta_y);
}

int main()
{
    Point p1 = { .x = 0.0, .y = 1.0 };
    Point p2 = { .x = 2.0, .y = 3.0 };
    double distance = calc_distance(p1, p2);
    printf("%f", distance);
    return 0;
}
```

## 面向对象

面向对象编程（OOP）试图用Object来组织和抽象程序的所有功能。Object封装了数据（Data Fields）和操作数据的函数（Member Functions）供外部调用。

很多现代编程语言都支持OOP，应用非常普遍。其中C++不仅支持OOP还支持过程式，而Java/.Net等语言从最顶层看就仅支持OOP一种编程范式，即使是作为程序入口的main函数也必须被包含在一个类中。

```csharp
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

函数式编程出现于上世纪五十年代，在面向数学运算的语言中运用较多。它的特点是视函数为“第一等公民（First Class）”，和普通数据变量拥有同等地位。

极少有完全函数式的语言在工业界广泛应用，函数式范式更多时候是通过lambda函数被非函数式语言局部运用的。限于篇幅，本节仅挑选[高阶函数（Higher Order Function）](http://www.shido.info/lisp/scheme8_e.html)这一个被很多非函数式的语言广泛吸收的特性。

所谓高阶函数，即接收函数作为参数的函数。

```lisp
; 自定义比较函数作为参数，传递给排序函数
(sort '(7883 9099 6729 2828 7754 4179 5340 2644 2958 2239) 
      (lambda (x y) (< (modulo x 100) (modulo y 100))))
⇒ (2828 6729 2239 5340 2644 7754 2958 4179 7883 9099)
```

高阶函数可以用来分离可变部分和不可变部分，从而提高代码的模块化程度和复用性。上面的例子中，排序函数关注排序算法本身，不关注比较函数如何实现，由用户提供具体的比较逻辑定制出不同的实现。

这一特性近来被大多数OOP语言吸收了，例如C++中的std::transform就是例子之一。

```c++
// 将字符原地转为大写
std::string s("hello");
std::transform(s.begin(), s.end(), 
               s.begin(),
               [](unsigned char c) -> unsigned char { return std::toupper(c); });
```

## 灵活运用编程范式

每种语言都有它支持的范式，还有更多它不支持的范式。这在一定程度上限制了使用某一特定语言时所能够拥有的“方法论”，例如用了C语言就不可能写出面向对象的Class。但编程范式提供的思路有时候是超越语言本身的。好的实践靠的是开发者心中对好的设计范式所提供思路的理解，而不是某种理论上的机械划分。

面向对象三大原则之一的“封装(Encapsulation)”，强调的是把数据和操作它们的函数放在一起。而C语言虽然不支持面向对象，但也不妨碍开发者贯彻这一思想，产出结构良好的代码。事实上，这一想法并不需要“封装”作为理论支撑，程序本来就是围绕一组数据（Data）进行的一系列操作（Code），增进它们之间的亲和性是理所应当的。面向对象的语言则是通过“类”这一概念施加了操作始终和某一对象（数据）绑定的约束。

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

函数式编程的高阶函数特性，强调的则是将函数作为参数传递，以提高模块化程度。但这不一定只有支持lambda的语言才做的到，C的函数指针（Function Pointer）一样办得到。

```c
unsigned char to_upper(unsigned char ch) 
{
    return std::toupper(ch);
}

void transform(unsigned char (*function)(unsigned char), unsigned char* array, int size) 
{
    for (int i = 0; i < size; i++) {
        array[i] = function(array[i]);
    }
}

int main() 
{
    unsigned char array[5] = {'h', 'e', 'l', 'l', 'o'};
    transform(to_upper, array, 5);
    return 0;
}
```

最后，在了解了一些编程范式的知识之后，回顾一些重要的事实：

* 编程语言的背后有一定的理论体系，每门语言都涵盖了一种或者多种编程范式。
* 构建程序的方式是多种多样的，每一门语言支持的范式只是众多范式中的一小部分。
