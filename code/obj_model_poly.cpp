class Left
{
public:
    virtual void print();
private:
    int y;
};

class Right
{
public:
    virtual void print();
private:
    int z;
};

class Bottom: public Left, public Right
{
public:
    void print();
private:
    int yz;
};

int main(int argc, char** argv) {
    return sizeof(Bottom);
}