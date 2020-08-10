class Top
{
public:
    void print_top();
private:
    int x;
};

class Left: public virtual Top
{
public:
    void print_left();
private:
    int y;
};

class Right: public virtual Top
{
public:
    void print_right();
private:
    int z;
};

class Bottom: public Left, public Right
{
public:
    void print_bottom();
private:
    int yz;
};

int main(int argc, char** argv) {
    return sizeof(Bottom);
}