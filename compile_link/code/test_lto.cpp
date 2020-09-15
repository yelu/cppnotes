// genreate so:
// g++ -O2 -flto test_lto.cpp -shared -o libtest_lto.so -fvisibility=hidden

#include "test_lto.h"

int mul(int m, int n) {
    int res = m*n;
    return res;
}

int add(int m, int n) {
    int res = m + n;
    return res;
}

int square_add(int m, int n) {
    int res = add(m*m, n*n);
    return res;
}