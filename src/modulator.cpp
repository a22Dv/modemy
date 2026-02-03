#include <climits>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numbers>
#include <stdexcept>

#include "modulator.hpp"

namespace mdm {

SineWaveGenerator::SineWaveGenerator(f32 sample_rate) : _sample_rate{sample_rate} {
}

void SineWaveGenerator::set_frequency(f32 frequency) {
    _cfreq = frequency;
    _csine_freq = f32(std::sinf(2.0f * f32(std::numbers::pi) * frequency / _sample_rate));
    _ccos_freq = f32(std::cosf(2.0f * f32(std::numbers::pi) * frequency / _sample_rate));
}

f32 SineWaveGenerator::next_sample() {
    const f32 sval = _csine;
    const f32 nsine = _csine * _ccos_freq + _ccos * _csine_freq;
    const f32 ncos = _ccos * _ccos_freq - _csine * _csine_freq;
    const f32 crct = (nsine * nsine + ncos * ncos) - 1;
    _csine = nsine * (1 - 0.5f * crct);
    _ccos = ncos * (1 - 0.5f * crct);
    return sval;
}

Modulator::Modulator(
    const std::filesystem::path &path, i32 sample_rate, i32 baud_rate, i32 high_tone, i32 low_tone
) :
    _generator{SineWaveGenerator{f32(sample_rate)}},
    _baud_rate{baud_rate},
    _sample_rate{sample_rate},
    _high_tone{high_tone},
    _low_tone{low_tone} {
    _stream = std::ifstream(path, std::ios::binary);
    if (!_stream.is_open()) {
        throw std::runtime_error("[ERROR] File stream initialization failure.");
    }
    if (_sample_rate % baud_rate != 0) {
        throw std::runtime_error("[ERROR] Incompatible baud rate with sample rate.");
    }
    _samples_per_baud = _sample_rate / _baud_rate;
}

f32 Modulator::get_sample() {
    if (_eof) {
        return 0.0f;
    }
    if (!_stream_counter) {
        if (_stream.eof()) {
            _eof = true;
            _current_samples_sent = 0;
            return 0.0f;
        }
        _stream.read(_data_staging.data(), (modulator_buffer_size * CHAR_BIT) / 10);
        const usize bytes_received = _stream.gcount();
        const usize net_bit_count = _insert_frame_bits(bytes_received);
        _stream_counter = net_bit_count * _samples_per_baud;
        _stream_buffer_idx = 0;
    }
    bool bit = (_data[_stream_buffer_idx] >> _stream_bit_position) & 1;
    if (bit != _mode) {
        _generator.set_frequency(f32(bit ? _high_tone : _low_tone));
        _mode = bit;
    }
    const f32 progression_factor = f32(_current_samples_sent) / _samples_per_baud;
    const f32 x = progression_factor - 0.5f;
    const f32 adj_amp = 1 - 16 * x * x * x * x;
    const f32 sample = _generator.next_sample() * adj_amp;

    ++_current_samples_sent;
    if (_current_samples_sent == _samples_per_baud) {
        _current_samples_sent = 0;
        _stream_buffer_idx += (_stream_bit_position + 1) == CHAR_BIT;
        _stream_bit_position = (_stream_bit_position + 1) % CHAR_BIT;
    }
    --_stream_counter;
    return sample;
}

bool Modulator::eof() const {
    return _eof;
}

usize Modulator::_insert_frame_bits(usize cbytes) {
    usize byte_idx = 0;
    u32 packet = 0;
    u32 remaining = 0;
    for (usize i = 0; i < cbytes; ++i) {
        const u8 byte = _data_staging[i];
        packet |= ((u32(byte) << 1) | 0x200) << remaining;
        remaining += 10;
        while (remaining >= 8) {
            _data[byte_idx] = packet & 0xFF;
            remaining -= 8;
            packet >>= 8;
            ++byte_idx;
        }
    }
    if (remaining > 0) {
        _data[byte_idx] = packet & 0xFF;
    }
    return cbytes * 10;
}

}  // namespace mdm