# Variadic Template

可变参数模板是C++11中引入的新特性。在这之前，定义一个支持可变参数个数的模板函数需要进行枚举：

```C++
template<typename T1>
std::string
format_str(const char* format, const T1& t1);

template<typename T1, typename T2>
std::string
format_str(const char* format, const T1& t1, const T2& t2);
```

使用可变参数模板时，通常还会结合利用编译器进行递归展开（meta pragramming常用技巧）。事实上，编译器帮助我们生成了一系列函数，它们分别接收不同个数的模板参数。

```C++
template<typename T>
std::string
format_str(boost::format& format, const T& t)
{
    return format % t;
}

template<typename T, typename... Rest>
std::string
format_str(boost::format& format, const T& t, const Rest&... rest)
{
    std::string tmp = format % t;
    return format_str(tmp, rest...);
}
```
