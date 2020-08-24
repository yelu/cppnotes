# 工厂模式

现实中，工厂负责生产产品。代码里，工厂模式负责创建（商品）类的实例化对象。对象的构造是由构造函数完成的，工厂方法允许调用者不直接接触类的构造函数，而是通过某种类型标识间接创建对应类型的对象。在设计模式里，这种模式被称为“简单工厂模式”。工厂模式对商品类的基本要求是它们需要有共同的基类。

对于支持反射的语言来说，简单工厂模式被语言直接提供了。

```csharp
// C#
public object CreateInstance(string typeName)
{         
    Type t = Type.GetType(typeName);
    return  Activator.CreateInstance(t);
}

```

C++是不支持反射的语言，做到这一点需要逐个对商品类型进行条件判断。

```cpp
// File: factory.cpp
Object* Factory::create_object(string type_name) {
    if (type_name == "Santana") {
        return new Santana();
    }
    else if(type_name == "Toyota") {
        return new Toyota();
    }
    else {
        return nullptr;
    }
}
```

对工厂模式的一个疑问是，为什么我们不能调用构造函数，非要用一个工厂来创建呢？导致类的构造函数不可用的原因有很多，比如对象被序列化到了磁盘文件上，后续通过文件反序列化时，必然面临通过类型标识创建对象的需求。面临这一问题的多是一些可扩展的框架或者插件系统。

## 类型注册

简单工厂模式就像它的名字，简单直接，易于理解，一眼就能望见所有实现细节，在一些轻量级的应用中，应该优优先考虑采用。

在简单工厂模式中，每增加一个新的“商品”种类，需要添加新的分支判断逻辑，导致工厂类的源代码需要频繁被修改。对一个需要持续维护和扩展的框架来说，这确实不是个特别好的设计。为了缓解问题，需要更加动态的类型注册机制。

第一步，用一个字典保存类型字符串到构造函数的映射。实现上的一点要求是，对象的创建函数需要绑定到一个统一的签名上去，例如下面的`Object* ()`。

```cpp
// File: factory.cpp
using Ctor = std::function<Object*()>;
unordered_map<string, Ctor> ctor_dict;

void reg_type(const string& type_name, Ctor ctor) {
    ctor_dict[type_name] = ctor;
}

Object* create(const string& type_name) {
    const auto& ite = ctor_dict.find(type_name);

    if (ite != ctor_dict.end()) {
        return (*(ite->second))();
    } else {
        throw runtime_error("type is not registered");
    }
}
```

第二步，注册代码需要脱离工厂类，最好由商品类自己源文件中的代码负责注册自己。为了在代码进入main函数之前触发商品类注册操作，常用的手法是利用静态全局对象的初始化机制间接完成。

```cpp
// File: santana.h
class Santana: public Object {
public:
    Santana() {}
}

// File: santana.cpp
#include "santana.h"

struct AutoRegister {
    AutoRegister(const string& type_name, Ctor ctor)
    {
        reg_type(type_name, ctor);
    }
};

static AutoRegister auto_reg("Santana", [](){ new Santana(); });
```

为了简化机械重复的注册逻辑，一般会将上面的注册代码定义成宏放回工厂类中。

```cpp
// File: factory.h
#define REGISTER_TYPE(type_name, ctor) \
    struct AutoRegister { \
        AutoRegister(const string& type_name, Ctor ctor) \
        { \
            reg_type(type_name, ctor); \
        } \
    }; \
    \
    static AutoRegister prod_type_##T##(type_name, ctor);

// File: santana.cpp
#include "factory.h"
#include "santana.h"

REGISTER_TYPE("Santana", [](){ new Santana(); });
```

全局变量的初始化顺序是该方法面临的一个风险。静态全局对象`auto_reg`初始化时会调用函数`reg_type`，该函数会访问另一个全局变量`ctor_dict`。跨编译单元的静态变量初始化顺序在标准中是未定义的，取决于编译器实现。虽然现在大多数编译器可能可以处理这种情况，更安全的做法是利用`Construct On First Use Idiom`将ctor_dict定义为局部静态变量并通过函数访问。

```cpp
// File: factory.cpp
unordered_map<string, Ctor>& get_ctor_dict() {
    static unordered_map<string, Ctor> ctor_dict;
    return ctor_dict;
}

void reg_type(const string& type_name, Ctor ctor) {
    get_ctor_dict()[type_name] = ctor;
}
```

到这里，我们成功分离了工厂模式的两个关键部分：工厂管理类和商品类注册。在增加新的商品类时工厂类代码能够保持不变，商品类的注册逻辑也分散在了每个商品类自身的源文件当中。

## 利用继承注册

Bartek在一篇blog post里[[cache]](Bartek_Factory_With_Self_Registering_Types.html)[[link]](https://www.bfilipek.com/2018/02/factory-selfregister.html)提出了另一种更加自动的注册机制，这种机制优雅的地方在于它将注册动作隐藏在了一个基类当中，只要继承就可以自动被注册。这个方法背后涉及更多的技术细节，采用与否需要进行权衡。

```cpp
// File: factory.h
template<typename T>
class AutoRegister {
  protected:
    static bool registered;
    virtual ~AutoRegister() {
        (void)registered;
    } // <-- just to prevent optimization

    static Object* create_object() { 
        return new T(); 
    };
};

// static member of template class should be initialized
// in hear file, not source file.
template<typename T>
bool AutoRegister<T>::registered = reg_type(
    T::type_name(), 
    &(AutoRegister<T>::create_object));

// File: santana.h
class Santana: 
    public Object,
    public AutoRegister<Santana> {
public:
    Santana() {}
    static std::string type_name() {
        return string("Santana");
    };
}
```

## 从独立的动态库中注册

理论上讲，如果将商品类编译为一个独立的动态库，无需任何变动，动态库加载时就可以做到自动注册商品类。不过，这中间涉及到了跨二进制边界调用函数时的ABI兼容性问题，除非动态库和工厂类都是受控的，即可以保证经由相同的编译器和编译参数编译。

目前，关于跨边界的函数调用，我们只涉及到了和注册、创建对象相关的两个函数。把它们导出为C APIs可以避免兼容性问题。

* 工厂类的类型注册函数`reg_create`，会被商品动态库调用。
* 商品类的创建函数`Object* ();`，会被工厂类调用。

但是，商品对象创建之后肯定还会存在调用它的其它接口的情况，这些函数调用同样存在问题。考虑到这些函数的时候，工厂模式就成了事实上的**插件系统**。关于构建插件系统的问题和方案，会在专门的一节中讨论。

## 练习

**1.** 使用`typeid(object).name()`作为类型注册时的key需要注意什么问题？

它的返回值是编译器实现相关的，标准没有规定返回的类型字符串的格式。在不同的可执行文件之间调用（跨二进制边界）时，如果它们是由不同的编译器或参数编译的，可能会出现不一致的问题。

## References

**1.** Self Registering Classes - Taking polymorphism to the limit[[cache]](ACCU_Self_Registering_Classes.html)[[link]](https://accu.org/index.php/journals/597)