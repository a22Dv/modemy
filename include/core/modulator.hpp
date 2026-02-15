#pragma once

#include <array>
#include <filesystem>
#include <fstream>
#include <numbers>
#include <span>
#include <utility>

#include "core/types.hpp"
#include "core/utils.hpp"

namespace mdm {

class ComplexOscillator {
   public:
    ComplexOscillator(i32 sample_rate, i32 frequency) :
        _delta{2.0 * std::numbers::pi * frequency / sample_rate},
        _sind{std::sinf(_delta)},
        _cosd{std::cosf(_delta)} {};
    std::pair<f64, f64> sin_cos() {
        const f64 nsin = _sin * _cosd + _cos * _sind;
        const f64 ncos = _cos * _cosd - _sin * _sind;
        const f64 correction = 1.0 - (0.5 * (nsin * nsin + ncos * ncos));
        _sin = nsin * correction;
        _cos = ncos * correction;
        return {_sin, _cos};
    }
   private:
    f64 _sin = 0.0;
    f64 _cos = 1.0;
    f64 _delta = 0.0;
    f64 _sind = 0.0;
    f64 _cosd = 0.0;
};

class Modulator {
   public:
    Modulator(const std::filesystem::path &filepath) {
        constexpr err_str ioerr = "[ERR] Failed to open file.";
        _file = std::ifstream{filepath, std::ios::binary};
        require(_file.is_open(), ioerr);
    }
    void get_samples(std::span<f32> samples) {

    }
    bool eof() const noexcept {
        return _eof;
    }
   private:
    std::ifstream _file;
    std::array<std::byte, 128> bytes{};
    bool _eof = false;
};

}  // namespace mdm