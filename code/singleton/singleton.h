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
