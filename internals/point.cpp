#include <stdio.h>      /* printf */
#include <math.h>       /* sqrt */

class Point
{
public:
    // Member Functions
    Point(double x, double y) {
        this->x = x;
        this->y = y;
    }
    double calc_distance(Point* p) {
        return 0.0;
    }
private:
    // Data Fields
    double x;
    double y;
};

int main() {
    Point p(1.0, 2.0);
    double d = p.calc_distance(&p);
    printf("%f", d);
    return 0;
}