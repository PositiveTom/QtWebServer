#include <iostream>

int main(int argc, char* argv[]) {
    std::array<char, 10> buf;
    size_t n = snprintf(buf.data(), buf.size()-1, "%d%dkasjdhadgagd", 7,9);
    std::cout << n << std::endl;

    for(int i=0; i<10; i++) {
        std::cout << "-" << buf[i] << "-" << std::endl;
    }
    return 0;
}