#include <iostream>

#include "audio.hpp"
#include "modulator.hpp"
#include "types.hpp"

/**
 * TODO:
 *
 * Implementation of:
 * - Modulator
 * - File handler
 * - Demodulator
 */

int main() {
    std::cout << "Hello World!\n";
    mdm::AudioDevice dev{44100};
    std::vector<mdm::i16> samples_s16le(24000, 0);
    std::vector<mdm::f32> samples_f32(12000, 0.0f);
    std::vector<mdm::f32> samples_f32_in(12000, 0.0f);

    // std::ifstream stream(, std::ios::binary);
    // stream.seekg(0x4E);

    mdm::Modulator modulator{"c:/Users/VBKLPTPA22/Downloads/ado.wav", 48000, 300, 1700, 1300};
    while (!modulator.eof()) {
        for (auto& f : samples_f32) {
            f = modulator.get_sample();
        }
        dev.write(samples_f32);
    }
    // while (!stream.eof()) {
    //     stream.read(reinterpret_cast<char *>(samples_s16le.data()), samples_s16le.size() * sizeof(mdm::i16));
    //     for (mdm::usize i = 0; i < samples_f32.size(); ++i) {
    //         samples_f32[i] = mdm::f32(samples_s16le[i * 2]) / INT16_MAX;
    //     }
    //     dev.write(samples_f32);
    // }
    return 0;
}