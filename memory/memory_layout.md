# 内存布局

## 进程地址空间

以上对象在一个进程的地址空间中如何摆放，是由操作系统和编译器联合决定的。通常情况下，在[C/C++的内存](https://www.geeksforgeeks.org/memory-layout-of-c-program/)分为以下几个区域:

* Stack segment。由编译器自动分配释放。
* Heap segment。由程序员分配释放。
* Data segment。初始化的全局变量和静态变量。
* BSS(Block Started by Symbol) segment。未初始化的全局变量和静态变量。
* Text segment。代码。

![Memory Layout for x86 and C](pic/memoryLayoutC.jpg)

```cpp
int a = 0; // Data
char *p1; // BSS
void main()
{
    int b; // Stack
    // string literals may be in Data(OS/compiler dependent)
    char *p3 = "123456"; 
    static int c = 0; // Data
    p1 = (char *)malloc(10); // Heap
}
```
