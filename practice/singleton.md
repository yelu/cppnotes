# Singleton

## 使用类静态成员

```cpp
// File: singleton.h
#include <memory>
#include <mutex>
#include <iostream>
using namespace std;

class Singleton {
public:
    static shared_ptr<Singleton> get_instance();

private:
    Singleton() { cout << "singleton instance created." << endl; }
    static shared_ptr<Singleton> _instance;
    static std::mutex _mtx;
};

// File: singletion.cpp
#include "singleton.h"

shared_ptr<Singleton> Singleton::_instance = nullptr;
std::mutex Singleton::_mtx;

shared_ptr<Singleton> Singleton::get_instance() {
    if(!_instance) {
        std::lock_guard<std::mutex> lock(_mtx);
        if(!_instance) {
            _instance.reset(new Singleton());
        }
    }
    return _instance;
}
```

## 利用局部静态变量

```cpp
Singleton& Singleton::get_instance() {   
    static Singleton instance;
    return instance;
}
```