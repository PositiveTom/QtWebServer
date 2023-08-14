#include <iostream>
#include <array>
class A {
public:
    A() {
        std::cout << "a" << std::endl;
    }
    ~A() {
        std::cout << "~a" << std::endl;
    }
};

class B : public A {
public:
    B() {
        std::cout << "b" << std::endl;
    }

    ~B() {
        std::cout << "~b" << std::endl;
    }
};


int main(int argc, char* argv[]) {

    B b;

    // std::array<char, 10> buf;
    // size_t n = snprintf(buf.data(), buf.size()-1, "%d%dkasjdhadgagd", 7,9);
    // std::cout << n << std::endl;

    // for(int i=0; i<10; i++) {
    //     std::cout << "-" << buf[i] << "-" << std::endl;
    // }
    return 0;
}