#include <iostream>

int main() {
    std::cout << "Hello World from Raspberry Pi 3!" << std::endl;
    return 0;
}


/*
build steps:
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../raspberrypi-toolchain.cmake
make
*/
