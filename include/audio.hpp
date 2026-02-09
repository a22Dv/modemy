#pragma once

#include <miniaudio.h>

#include <atomic>
#include <span>
#include <vector>


#include "types.hpp"

namespace mdy {

/// @brief Audio device. Duplex.
class AudioDevice {
   public:
    AudioDevice(i32 sample_rate = 48000, i32 channels = 2);
    AudioDevice(const AudioDevice &) = delete;
    AudioDevice operator=(const AudioDevice &) = delete;
    AudioDevice(AudioDevice &&) = delete;
    AudioDevice operator=(AudioDevice &&) = delete;
    ~AudioDevice();
    void read(std::span<f32> samples);
    void write(std::span<const f32> samples);

   private:
    static constexpr usize _internal_queue_size_limit = 512;
    static void _data_callback(
        ma_device *device, void *out, const void *in, ma_uint32 sample_count
    );
    ma_device _device{};
    std::vector<f32> _sample_rqueue{};
    std::atomic<usize> _squeue_rmain_idx{};
    std::atomic<usize> _squeue_rbkg_idx{};
    std::vector<f32> _sample_wqueue{};
    std::atomic<usize> _squeue_wmain_idx{};
    std::atomic<usize> _squeue_wbkg_idx{};
};

}  // namespace mdy