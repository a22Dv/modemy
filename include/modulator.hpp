#pragma once

#include <array>
#include <filesystem>
#include <fstream>

#include "types.hpp"

namespace mdm {

constexpr usize modulator_buffer_size = 200;

/// @brief Utility class, primarily for use within the Modulator class' implementation details.
class SineWaveGenerator {
   public:
    SineWaveGenerator(f32 sample_rate);

    /// @brief Sets the frequency of the sine wave generated.
    /// @note This is a CFSK generator.
    void set_frequency(f32 frequency);

    /// @brief Get the next sine wave sample.
    f32 next_sample();

   private:
    f32 _cphase = 0.0f;
    f32 _cfreq = 0.0f;
    f32 _sample_rate = 0.0f;
    f32 _csine = 0.0f;
    f32 _ccos = 1.0f;
    f32 _ccos_freq = 0.0f;
    f32 _csine_freq = 0.0f;
};

/// @brief Modulates a given file into audio samples.
class Modulator {
   public:
    Modulator(const std::filesystem::path &path, i32 sample_rate, i32 baud_rate, i32 high_tone, i32 low_tone);
    Modulator(const Modulator &) = delete;
    Modulator operator=(const Modulator &) = delete;
    Modulator(Modulator &&) = delete;
    Modulator operator=(Modulator &&) = delete;
    ~Modulator() = default;

    /// @brief Call in a loop to get the next sample.
    /// @note Check eof() to check for end-of-file. Always returns 0.0 if eof is reached.
    f32 get_sample();

    /// @brief End-of-file status.
    bool eof() const;
   private:

    /// @brief Internal implementation detail. Returns number of bits.
    usize _insert_frame_bits(usize cbytes);
    SineWaveGenerator _generator;
    std::array<char, modulator_buffer_size> _data = {};
    std::array<char, modulator_buffer_size> _data_staging = {};
    std::ifstream _stream = {};
    usize _stream_buffer_idx = 0;
    usize _stream_bit_position = 0;
    usize _stream_counter = 0;

    i32 _baud_rate = 0;
    i32 _sample_rate = 0;
    i32 _high_tone = 0;
    i32 _low_tone = 0;
    i32 _current_samples_sent = 0;
    i32 _samples_per_baud = 0;
    bool _mode = true;
    bool _eof = false;
};

}  // namespace mdm