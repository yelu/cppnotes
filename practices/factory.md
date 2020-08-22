# 工厂模式

[Factory With Self-Registering Types](https://www.bfilipek.com/2018/02/factory-selfregister.html)

现实中，工厂负责生产产品。程序世界里，工厂模式负责创建类的实例化对象。类的构造是由其构造函数完成的，工厂方法允许调用者不接触类的构造函数，通过特定的标识创建对应类型的对象。在设计模式里，这种模式被称为“简单工厂模式”。

这个问题对于支持反射的语言来说，几乎不算是个问题，简单工厂模式被语言本身直接提供了。

```csharp
public object CreateInstance(string className)
{         
    Type t = Type.GetType(className);
    return  Activator.CreateInstance(t);
}

```

C++是不支持反射的语言，要做到这一点就需要对一个个类型进行条件判断。

```cpp
// 简单工厂
```

简单工厂经常被诟病的一点是，没次增加一个新的“商品”种类，都需要修改工厂代码，于是更“高级”的工厂模式产生了。但是，这种简单直接的模式实际上非常易于理解，一眼就能望见所有实现细节，对于更新工厂方法不介意的场景是应该优优先考虑采用的。

对工厂方法的一个疑问是，为什么我们不能调用构造函数，非要用一个工厂来创建呢？这里面很重要的一个应用场景是一些可扩展的框架。这些框架的设计通常有一个很重要的问题需要解决，那就是通过字符串来创建类对象。导致类的构造函数不可用的原因有很多，比如类对象被序列化到了磁盘上，后续需要通过文件中的字符串来恢复对象。

## 类型注册

以上分支判断实现的一个不好的地方是，在增加新的类型时需要不停地添加新的分支判断代码。为了缓解这一问题代码的麻烦，可以用一个字典保存类型字符串到构造函数的映射表。

```cpp
typedef Object* (*Ctor)();
std::unordered_map<std::string, Ctor> ctor_dict;

void register(const char* type_name, Ctor ctor)
{
    ctor_dict[type_name] = ctor;
}

Object* create(const std::string& type_name)
{
    const auto& ite = ctor_dict.find(type_name);

    if (ite != ctor_dict.end()) {
        return (*(ite->second))();
    } else {
        throw std::runtime_error("class is not registered");
    }
}
```

这一实现需要被创建的工厂类慢则一定的要求：

* 有共同的基类。这是所有工厂方法的共同要求。
* 构造函数签名一致。

## 自动注册

对于一个需要被不停地维护和扩展的框架来说，因为添加新的类型，经常需要修改工厂类本身的实现确实不是个特别好的设计。如果能让注册代码脱离于工厂类之外，是更好的选择。

想要在代码进入main函数之前注册好所有工厂类，需要利用静态全局对象的初始化来间接完成。

```cpp
struct AutoRegister
{
    AutoRegister(const char* type_name, Ctor ctor)
    {
        register(type_name, ctor);
    }
};

class Santana {
public:
    Santana() {}
}

static AutoRegister auto_reg("Santana", Santana::Santana);
```

为了简化机械重复的注册逻辑，一般会通过定义宏的方式实现。为了支持在同一个源文件中注册多个类型，还需要处理一下变量名等细节。

```cpp
struct AutoRegister
{
    AutoRegister(const char* type_name, Ctor ctor)
    {
        register(type_name, ctor);
    }
};

class Santana {
public:
    Santana() {}
}

#define REGISTER_TYPE_NAME(T) reg_type_##T##_
#define REGISTER_TYPE(type_name, T) static AutoRegister REGISTER_TYPE_NAME(T)(type_name, T::T);

REGISTER_TYPE("Santana", Santana::Santana);
```

Bartek在[blog](Bartek_Factory_With_Self_Registering_Types.html)中提出了另一种更加自动的注册机制，这种机制优雅的地方在于它将注册动作隐藏在了一个基类当中，只要继承就可以自动被注册。这个方法对用户比较友好，不过背后涉及了稍多一些的细节，采用需要权衡。

```cpp

template<typename T>
class AutoRegister
{
  protected:
    static bool registered;
    virtual ~AutoRegister()
    {
        (void)registered;
    } // <-- just to prevent the optimization

    static Operator* CreateInstance() { return new T(); };
};

template<typename T>
bool AutoRegister<T>::registered =
  OpFactory::Register(typeid(T).name(), T::GetTypeName().c_str(), &(AutoRegister<T>::CreateInstance));
```

## 作为插件注册

[build a plugin system](https://sourcey.com/articles/building-a-simple-cpp-cross-platform-plugin-system)

## 练习

**1.** 使用`typeid(object).name()`作为类型注册时的key需要注意什么问题？

它的返回值是编译器实现相关的，标准没有规定返回的类型字符串的格式。在不同的可执行文件之间调用（跨二进制边界）时，会出现问题。
