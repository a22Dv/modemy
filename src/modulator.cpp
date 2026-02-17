#include <climits>
#include <cmath>
#include <iostream>
#include <span>
#include <utility>

#include "core/modulator.hpp"
#include "core/types.hpp"
#include "core/utils.hpp"

namespace {
using namespace mdm;

f64 _scale_to_freq(Scale scale, i32 step, f64 root_frequency) {
    constexpr err_str invaliderr = "[ERR] Invalid argument.";
    require(step >= 0 && step < _bin_widths[usize(scale)], invaliderr);
    return root_frequency * std::pow(2.0, _steps[usize(scale)][step] / 12.0);
}

}  // namespace

namespace mdm {

usize Modulator::get_samples(std::span<f32> samples) {
    if (_eof && !_bitw_remaining) {
        return 0;
    }
    const usize bin_size = _bin_widths[usize(_scale)];
    const usize sample_count = samples.size();

    usize smp_written = 0;
    for (usize i = 0; i < sample_count;) {
        if (_periods == _period_samples) {
            _bitw_remaining = _bitw_remaining < bin_size ? 0 : _bitw_remaining - bin_size;
            _bit_window >>= bin_size;
            _periods = 0;
        }
        if (!_bytes_remaining && !_eof) {
            _datastream.read(reinterpret_cast<char *>(_databuffer.data()), _databuffer.size());
            _bytes_remaining = _datastream.gcount();
            _cbyte = 0;
            if (_datastream.eof()) {
                _eof = true;
            }
        }
        while (_bitw_remaining < bin_size && _bytes_remaining) {
            _bit_window |= u64(_databuffer[_cbyte]) << _bitw_remaining;
            --_bytes_remaining;
            _bitw_remaining += CHAR_BIT;
            ++_cbyte;
        }
        if (!_bitw_remaining) {
            break;
        }
        f32 sample = 0.0f;
        for (usize j = 0; j < bin_size; ++j) {
            const i32 target = (_bit_window >> j) & 1;
            const f32 vch = f32(_oscillators[j].values().first);
            const f32 vchamp = vch * _crnt_amplitudes[j];
            const f32 ampch = ((target * _max_amplitudes[j]) - _crnt_amplitudes[j]) * _window_delay;
            _crnt_amplitudes[j] += ampch;
            sample += vchamp;
        }
        sample /= _max_amp_sum;
        i32 chsmp = 0;
        for (i32 j = 0; j < _channels && i < sample_count; ++j) {
            // Might cut off if sample buffer is not a multiple of _channels.
            samples[i++] = sample;
            ++chsmp;
        }
        ++_periods;
        smp_written += chsmp;
    }
    return smp_written;
}

Modulator::Modulator(
    f32 root_frequency,
    f32 bpm,
    Scale scale,
    i32 sample_rate,
    i32 channels,
    const std::filesystem::path &filepath
) :
    _root_freq{root_frequency},
    _bpm{i32(bpm)},
    _sample_rate{sample_rate},
    _channels{channels},
    _scale{scale} {
    constexpr err_str ioerr = "[ERR] Failure to open file.";
    constexpr err_str inverr = "[ERR] Invalid argument.";

    _datastream = std::ifstream{filepath, std::ios::binary};
    require(_datastream.is_open(), ioerr);
    const i32 period_samples = i32(sample_rate / (bpm / 60.0));
    require(period_samples >= 1, inverr);
    _datastream.read(reinterpret_cast<char *>(_databuffer.data()), _databuffer.size());
    _bytes_remaining = _datastream.gcount();
    _period_samples = period_samples;
    _window_delay = 1.0f / (0.02f * sample_rate);

    const usize bin_size = _bin_widths[usize(scale)];
    _oscillators.resize(bin_size);
    _max_amplitudes.resize(bin_size);
    _crnt_amplitudes.resize(bin_size);
    for (usize i = 0; i < bin_size; ++i) {
        const f64 freq = _scale_to_freq(scale, i32(i), root_frequency);
        ComplexOscillator cxosc{freq, f64(sample_rate)};
        _oscillators[i] = std::move(cxosc);
        const f64 amp = 1.0 / freq;
        _max_amplitudes[i] = f32(amp);
        _max_amp_sum += f32(amp);
    }
    std::fill(_crnt_amplitudes.begin(), _crnt_amplitudes.end(), 0.0f);
}

}  // namespace mdm