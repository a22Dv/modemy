#pragma once

extern "C" {
#include <external/miniaudio.h>
}

#include <span>

#include "core/spsc_queue.hpp"
#include "core/types.hpp"
#include "core/utils.hpp"

namespace mdm {

class AudioDevice {
   public:
    AudioDevice(i32 sampleRate, i32 channels, bool in, bool out) {
        constexpr err_str ierr = "[ERR] Device initialization failure.";
        constexpr err_str serr = "[ERR] Device startup failure.";
        ma_device_type type = ma_device_type((i32(in) << 1) | i32(out));
        ma_device_config cfg = ma_device_config_init(type);
        cfg.sampleRate = sampleRate;
        if (in) {
            cfg.capture.channels = channels;
            cfg.capture.format = ma_format_f32;
        }
        if (out) {
            cfg.playback.channels = channels;
            cfg.playback.format = ma_format_f32;
        }
        cfg.dataCallback = _background_thread_exec;
        cfg.pUserData = this;
        ma_result mir = ma_device_init(nullptr, &cfg, &device);
        require(mir == MA_SUCCESS, ierr);
        ma_result msr = ma_device_start(&device);
        require(msr == MA_SUCCESS, serr);
    }

    ~AudioDevice() {
        ma_device_uninit(&device);
    }

    usize push(std::span<const f32> samples) {
        return wqueue.push<true>(samples);
    }

    usize pop(std::span<f32> samples_buffer) {
        return rqueue.pull<true>(samples_buffer);
    }

   private:
    static void _background_thread_exec(ma_device *dev, void *out, const void *in, ma_uint32 sc) {
        AudioDevice &device = *static_cast<AudioDevice *>(dev->pUserData);
        const usize tscr = sc * device.device.capture.channels;
        const usize tscw = sc * device.device.playback.channels;
        const std::span<const f32> sp_in{static_cast<const f32 *>(in), tscr};
        const std::span<f32> sp_out{static_cast<f32 *>(out), tscw};
        device.rqueue.push<false>(sp_in);
        const usize spo = device.wqueue.pull<false>(sp_out);
        if (spo != tscw) {
            std::fill(sp_out.begin() + spo, sp_out.end(), 0.0f);
        }
    }
    SPSCQueue<1024, f32> rqueue{};
    SPSCQueue<1024, f32> wqueue{};
    ma_device device{};
};

}  // namespace mdm