#define NOMINMAX
#define MINIAUDIO_IMPLEMENTATION
extern "C" {
#include <miniaudio.h>
};

#include <iostream>

#include "audio.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    std::cout << "Hello World!\n";
    return 0;
}
