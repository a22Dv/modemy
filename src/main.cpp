#pragma warning(push, 1)
#define MINIAUDIO_IMPLEMENTATION
extern "C" {
#include <external/miniaudio.h>
}
#pragma warning(pop)

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "core/audio_device.hpp"
#include "core/modulator.hpp"

int main(int argc, char **argv) {
    using namespace mdm;
    try {
        if (argc != 7) {
            std::cout
                << "Usage: modemy <SAMPLE_RATE> <CHANNELS> <ROOT-FREQUENCY> <MUSICAL SCALE> <BPM> "
                   "<FILE-PATH>\n";
            return EXIT_FAILURE;
        }
        const f32 sample_rate = std::stof(argv[1]);
        const i32 channels = std::stoi(argv[2]);
        const f32 root_frequency = std::stof(argv[3]);
        const Scale scale = [&] {
            std::string scl = argv[4];
            std::transform(scl.begin(), scl.end(), scl.begin(), [](auto a) {
                return char(a & ~0x20);
            });
            if (scl == "MAJOR") {
                return Scale::MAJOR;
            } else if (scl == "MINOR") {
                return Scale::MINOR;
            } else if (scl == "PENTATONIC") {
                return Scale::PENTATONIC;
            }
            throw std::runtime_error("[ERR] Invalid Argument.");
            return Scale::MINOR;
        }();
        const i32 bpm = std::stoi(argv[5]);
        const std::filesystem::path fpath{argv[6]};
        AudioDevice dev{i32(sample_rate), channels, false, true};
        Modulator mod{root_frequency, f32(bpm), scale, i32(sample_rate), channels, fpath};
        std::vector<f32> data(2048);
        while (!mod.eof()) {
            mod.get_samples(data);
            dev.push(data);
        }
        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
}