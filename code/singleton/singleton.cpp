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

/*
inline static Singleton& get_instance() {   
    static Singleton instance;
    return instance;
}*/

int main() {
    auto ins = Singleton::get_instance();
    Singleton::get_instance();
    return 0;
}

