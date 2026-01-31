#pragma once

extern "C" {
#include <miniaudio.h>
}

#include <new>
#include <span>
#include <vector>
#include <atomic>

#include "types.hpp"

#define MDM_ALIGNAS_HDI alignas(std::hardware_destructive_interference_size)

namespace mdm {

using f32 = float;

constexpr i32 sample_rate = 48000;
constexpr i32 channels = 2;

/// @brief Audio device that wraps miniaudio internals. Supports in/out.
class AudioDevice {
   public:
    /// @brief Read a given number of samples from the microphone for the full length of span.
    /// @note This is a blocking operation and will fill the given span.
    void read(std::span<f32> samples);

    /// @brief Write a given number of samples from the span to an internal buffer.
    /// @note This is a blocking operation, and exhausts the full span before returning.
    void write(std::span<const f32> samples);

   private:
    /// @brief Internal miniaudio callback.
    static void _callback(ma_device* device, void* out, void* in, ma_uint32 sample_count);

    ma_device _device{};
    MDM_ALIGNAS_HDI std::atomic<usize> _in_head = 0; // Writer.
    MDM_ALIGNAS_HDI std::atomic<usize> _out_head = 0; // Writer.
    MDM_ALIGNAS_HDI std::atomic<usize> _in_tail = 0; // Reader.
    MDM_ALIGNAS_HDI std::atomic<usize> _out_tail = 0; // Reader.
    MDM_ALIGNAS_HDI std::atomic<bool> _terminate = false;
    std::vector<f32> _out_samples{usize(sample_rate / 2), 0.0f};
    std::vector<f32> _in_samples{usize(sample_rate / 2), 0.0f};
};

}  // namespace mdm