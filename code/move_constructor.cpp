#include <string>
#include <iostream>
#include <iomanip>
#include <utility>
 
class A
{
public:
    std::string s;
    int k;
    A() : s("test"), k(-1) { std::cout << "default constructor!\n"; }
    A(const A& o) : s(o.s), k(o.k) { std::cout << "copy constructor!\n"; }
    /**
    A(A&& o) noexcept :
           s(std::move(o.s)),       // explicit move of a member of class type
           k(std::exchange(o.k, 0)) // explicit move of a member of non-class type
    { 
        std::cout << "move constructor!\n";
    }**/
};
 
A f1(A a)
{
    a.s = "hello";
    return a;
}

A f2()
{
    A a;
    a.s = "hello";
    return a;
}
  
int main()
{
    A a1 = f1(A()); // return by value move-constructs the target from the function parameter
    A a2 = f2();
}