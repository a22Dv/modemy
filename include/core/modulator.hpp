#pragma once

#include <array>
#include <filesystem>
#include <fstream>
#include <span>
#include <numbers>
#include <vector>

#include "core/types.hpp"

namespace mdm {

class ComplexOscillator {
   public:
    ComplexOscillator(f64 freq = 0.0, f64 sample_rate = 1.0) :
        _sind{std::sin(2 * std::numbers::pi * (freq / sample_rate))},
        _cosd{std::cos(2 * std::numbers::pi * (freq / sample_rate))},
        _freq{freq},
        _sample_rate{sample_rate} {};
    void set_frequency(f64 freq) {
        _freq = freq;
        _sind = std::sin(2 * std::numbers::pi * (freq / _sample_rate));
        _cosd = std::cos(2 * std::numbers::pi * (freq / _sample_rate));
    }
    std::pair<f64, f64> values() {
        const f64 ncos = _cosc * _cosd - _sinc * _sind;
        const f64 nsin = _cosc * _sind + _sinc * _cosd;
        const f64 crc = 1.5 - (0.5 * (ncos * ncos + nsin * nsin));
        _sinc = nsin * crc;
        _cosc = ncos * crc;
        return {_cosc, _sinc};
    }

   private:
    f64 _sinc = 0.0;
    f64 _cosc = 1.0;
    f64 _sind = 0.0;
    f64 _cosd = 0.0;
    f64 _freq = 0.0;
    f64 _sample_rate = 0.0;
};
enum class Scale {
    MAJOR,
    MINOR,
    PENTATONIC
};

static constexpr std::array _bin_widths = {8, 8, 6};
static constexpr std::array _step_major = {0, 2, 4, 5, 7, 9, 11, 12};
static constexpr std::array _step_minor = {0, 2, 3, 5, 7, 8, 10, 12};
static constexpr std::array _step_pentatonic = {0, 2, 4, 7, 9, 12};
static constexpr std::array _steps = {
    _step_major.data(), _step_minor.data(), _step_pentatonic.data()
};

class Modulator {
   public:
    Modulator(
        f32 root_frequency,
        f32 bpm,
        Scale scale,
        i32 sample_rate,
        i32 channels,
        const std::filesystem::path &filepath
    );
    usize get_samples(std::span<f32> samples);
    bool eof() const noexcept { return _eof; }
   private:
    std::vector<ComplexOscillator> _oscillators{};
    std::vector<f32> _max_amplitudes{};
    std::vector<f32> _crnt_amplitudes{};
    std::array<byte, 128> _databuffer = {};
    std::ifstream _datastream;
    u64 _bit_window = 0;
    u64 _bitw_remaining = 0;
    u64 _bytes_remaining = 0;
    u64 _cbyte = 0;
    u64 _period_samples = 0;
    u64 _periods = 0;
    f32 _window_delay = 0.0f;
    f32 _root_freq = 0.0f;
    f32 _max_amp_sum = 0.0f;
    i32 _bpm = 0;
    i32 _sample_rate = 0;
    i32 _channels = 0;
    Scale _scale;
    bool _eof = false;
};

}  // namespace mdm