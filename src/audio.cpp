#define MINIAUDIO_IMPLEMENTATION
#define NOMINMAX

#pragma warning(push, 1)
extern "C" {
#include <miniaudio.h>
}
#pragma warning(pop)

#include <algorithm>
#include <span>
#include <stdexcept>
#include <vector>

#include "audio.hpp"
#include "types.hpp"

namespace mdm {

AudioDevice::AudioDevice(i32 sample_rate) : _sample_rate{sample_rate} {
    _out_samples = std::vector<f32>(usize(_sample_rate / 4), 0.0f);
    _in_samples = std::vector<f32>(usize(_sample_rate / 4), 0.0f);
    ma_device_config config = ma_device_config_init(ma_device_type_duplex);
    config.capture.channels = channels;
    config.playback.channels = channels;
    config.capture.format = ma_format_f32;
    config.playback.format = ma_format_f32;
    config.sampleRate = _sample_rate;
    config.pUserData = this;
    config.dataCallback = _callback;
    ma_result initr = ma_device_init(NULL, &config, &_device);
    if (initr != MA_SUCCESS) {
        throw std::runtime_error("[ERROR] Device initialization failure.");
    }
    ma_result strtr = ma_device_start(&_device);
    if (strtr != MA_SUCCESS) {
        throw std::runtime_error("[ERROR] Device startup failure.");
    }
}

AudioDevice::~AudioDevice() {
    ma_device_stop(&_device);
    ma_device_uninit(&_device);
}

void AudioDevice::read(std::span<f32> samples) {
    using mem = std::memory_order;

    const usize sample_count = samples.size();
    const usize buffer_size = _in_samples.size();

    usize samples_read = 0;
    while (samples_read < sample_count) {
        const usize w_idx = _in_head.load(mem::acquire);
        const usize r_idx = _in_tail.load(mem::relaxed);
        const usize dist = (buffer_size * (w_idx < r_idx)) + w_idx - r_idx;
        if (w_idx == r_idx) {
            _in_head.wait(w_idx, mem::acquire);
            continue;
        }
        const usize read_count = std::min({sample_count - samples_read, dist, buffer_size - r_idx});
        std::copy_n(_in_samples.begin() + r_idx, read_count, samples.begin() + samples_read);
        _in_tail.store((r_idx + read_count) % buffer_size, mem::release);
        samples_read += read_count;
    }
}

void AudioDevice::write(std::span<const f32> samples) {
    using mem = std::memory_order;

    const usize buffer_size = _out_samples.size();
    const usize samples_count = samples.size();

    usize samples_written = 0;
    while (samples_written < samples_count) {
        const usize w_idx = _out_head.load(mem::relaxed);
        const usize r_idx = _out_tail.load(mem::acquire);
        if ((w_idx + 1) % buffer_size == r_idx) {
            _out_tail.wait(r_idx, mem::acquire);
            continue;
        }
        const usize available = buffer_size * (r_idx <= w_idx) + r_idx - w_idx;
        const usize write_count =
            std::min({available - 1, buffer_size - w_idx, samples_count - samples_written});
        std::copy_n(samples.begin() + samples_written, write_count, _out_samples.begin() + w_idx);
        _out_head.store((w_idx + write_count) % buffer_size, mem::release);
        samples_written += write_count;
    }
}

void AudioDevice::_callback(ma_device *device, void *out, const void *in, ma_uint32 sample_count) {
    using mem = std::memory_order;
    AudioDevice &adev = *static_cast<AudioDevice *>(device->pUserData);

    const usize out_buffer_size = adev._out_samples.size();
    const usize in_buffer_size = adev._in_samples.size();
    const f32 *inf = static_cast<const f32 *>(in);
    f32 *outf = static_cast<f32 *>(out);

    bool processed = false;
    usize samples_processed = 0;
    while (samples_processed < sample_count) {
        const usize out_w_idx = adev._out_head.load(mem::acquire);
        const usize out_r_idx = adev._out_tail.load(mem::relaxed);
        const usize available = out_buffer_size * (out_w_idx < out_r_idx) + out_w_idx - out_r_idx;
        if (!available) {
            std::fill(outf + samples_processed, outf + sample_count, 0.0f);
            break;
        }
        const usize write_count =
            std::min({available, out_buffer_size - out_r_idx, sample_count - samples_processed});
        std::copy_n(adev._out_samples.begin() + out_r_idx, write_count, outf + samples_processed);
        adev._out_tail.store((out_r_idx + write_count) % out_buffer_size, mem::release);
        samples_processed += write_count;
        processed = true;
    }
    if (processed) {
        adev._out_tail.notify_one();
    }

    processed = false;
    samples_processed = 0;
    while (samples_processed < sample_count) {
        const usize in_w_idx = adev._in_head.load(mem::relaxed);
        const usize in_r_idx = adev._in_tail.load(mem::acquire);
        const usize available = in_buffer_size * (in_r_idx <= in_w_idx) + in_r_idx - in_w_idx;
        if (!available) {
            break;
        }
        const usize read_count =
            std::min({available, in_buffer_size - in_w_idx, sample_count - samples_processed});
        std::copy_n(inf + samples_processed, read_count, adev._in_samples.begin() + in_w_idx);
        adev._in_head.store((in_w_idx + read_count) % in_buffer_size, mem::release);
        samples_processed += read_count;
        processed = true;
    }
    if (processed) {
        adev._in_head.notify_one();
    }
}

}  // namespace mdm