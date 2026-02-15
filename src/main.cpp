#include <fstream>
#include <iostream>
#include <vector>

#pragma warning(push, 1)
#define MINIAUDIO_IMPLEMENTATION
extern "C" {
#include <external/miniaudio.h>
}
#pragma warning(pop)

#include "core/audio_device.hpp"

int main() {
    std::ifstream s{"C:/dev/repositories/modemy/data/wh.wav", std::ios::binary};
    s.seekg(0x4E);
    mdm::AudioDevice dev{44100, 2, false, true};

    std::vector<mdm::i16> sm16(2048);
    std::vector<mdm::f32> sm32(2048);
    while (!s.eof()) {
        s.read(reinterpret_cast<char *>(sm16.data()), sm16.size() * sizeof(mdm::i16));
        for (mdm::usize i = 0; i < sm16.size(); ++i) {
            sm32[i] = sm16[i] / 32767.0f;
        }
        dev.push(sm32);
    }
    return 0;
}