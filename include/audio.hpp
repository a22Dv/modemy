#pragma once

extern "C" {
#include <miniaudio.h>
}

#include <atomic>
#include <new>
#include <span>
#include <vector>

#include "types.hpp"

#define MDM_ALIGNAS_HDI alignas(std::hardware_destructive_interference_size)

#pragma warning(push)
#pragma warning(disable : 4324)

namespace mdm {

using f32 = float;

constexpr i32 sample_rate = 44100;
constexpr i32 channels = 1;

/// @brief Audio device that wraps miniaudio internals. Supports in/out.
/// @note Non-copyable, non-movable.
class AudioDevice {
   public:
    AudioDevice();
    ~AudioDevice();

    AudioDevice(const AudioDevice &) = delete;
    AudioDevice operator=(const AudioDevice &) = delete;
    AudioDevice(AudioDevice &&) = delete;
    AudioDevice operator=(AudioDevice &&) = delete;

    /// @brief Read a given number of samples from the microphone for the full length of span.
    /// @note This is a blocking operation and will fill the given span.
    void read(std::span<f32> samples);

    /// @brief Write a given number of samples from the span to an internal buffer.
    /// @note This is a blocking operation, and exhausts the full span before returning.
    void write(std::span<const f32> samples);

   private:
    /// @brief Internal miniaudio callback.
    static void _callback(ma_device *device, void *out, const void *in, ma_uint32 sample_count);

    ma_device _device{};
    MDM_ALIGNAS_HDI std::atomic<usize> _in_head = 0;   
    MDM_ALIGNAS_HDI std::atomic<usize> _out_head = 0;  
    MDM_ALIGNAS_HDI std::atomic<usize> _in_tail = 0;   
    MDM_ALIGNAS_HDI std::atomic<usize> _out_tail = 0; 
    std::vector<f32> _out_samples = std::vector<f32>(usize(sample_rate / 4), 0.0f);
    std::vector<f32> _in_samples = std::vector<f32>(usize(sample_rate / 4), 0.0f);
};

}  // namespace mdm

#pragma warning(pop)