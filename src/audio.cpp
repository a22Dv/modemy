extern "C" {
#include <miniaudio.h>
}

#include "audio.hpp"
#include "utils.hpp"

namespace mdy {

AudioDevice::AudioDevice(i32 sample_rate, i32 channels) {
    constexpr err_msg init_err = "[ERROR] Device Initializaiton Failure\n";
    constexpr err_msg start_err = "[ERROR] Device Startup Failure\n";

    ma_device_config dconfig = ma_device_config_init(ma_device_type_duplex);
    dconfig.sampleRate = sample_rate;
    dconfig.pUserData = this;
    dconfig.dataCallback = _data_callback;
    dconfig.playback.format = ma_format_f32;
    dconfig.playback.channels = channels;
    dconfig.capture.format = ma_format_f32;
    dconfig.capture.channels = channels;
    require(ma_device_init(nullptr, &dconfig, &_device) == MA_SUCCESS, init_err);
    require(ma_device_start(&_device) == MA_SUCCESS, start_err);
}

AudioDevice::~AudioDevice() {
    ma_device_stop(&_device);
    ma_device_uninit(&_device);
}

void AudioDevice::read(std::span<f32> samples) {
    const usize samples_count = samples.size();

    usize sample_proc = 0;
    const auto sb = samples.begin();
    const auto qb = _sample_rqueue.begin();
    while (sample_proc < samples_count) {
        const usize ridx = _squeue_rmain_idx.load(std::memory_order_relaxed);
        const usize widx = _squeue_rbkg_idx.load(std::memory_order_acquire);
        const usize available = _internal_queue_size_limit * (ridx > widx) + widx - ridx;
        if (!available) {
            _squeue_rbkg_idx.wait(widx, std::memory_order_acquire);
            continue;
        }
        const usize samples_remaining = samples_count - sample_proc;
        const usize contiguous_remaining = _internal_queue_size_limit - ridx;
        const usize rcount = std::min({available, samples_remaining, contiguous_remaining});
        std::copy_n(qb + ridx, rcount, sb + sample_proc);
        sample_proc += rcount;
        _squeue_rmain_idx.store(
            (ridx + rcount) % _internal_queue_size_limit, std::memory_order_release
        );
    }
}

void AudioDevice::write(std::span<const f32> samples) {

}

void AudioDevice::_data_callback(
    ma_device *device, void *out, const void *in, ma_uint32 sample_count
) {
    AudioDevice &dev = *static_cast<AudioDevice *>(device->pUserData);
}

}  // namespace mdy