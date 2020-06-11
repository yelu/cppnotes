#include <cstdio>

#define SUM(a, b) (a+b)

int sum(int a, int b){
    return a + b;
}

int main(int argc, char* argv[]) {
    int res = sum(1, 2);
    printf("%d", res);
    res = SUM(1, 2);
    return 0;
}