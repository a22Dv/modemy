#pragma once

#include <span>
#include <thread>
#include <vector>

#include "types.hpp"

namespace mdm {

constexpr i32 window_size = 512;

class Demodulator {
   public:
    Demodulator();
    Demodulator(const Demodulator &) = delete;
    Demodulator operator=(const Demodulator &) = delete;
    Demodulator(Demodulator &&) = delete;
    Demodulator operator=(Demodulator &&) = delete;
    ~Demodulator();

    /// @brief Push audio samples to demodulator. Returns the number
    ///        of samples received. This function will block until all samples have
    ///        been put into the internal queue.
    void push_samples(std::span<const f32> samples);

    /// @brief Returns the number of samples written to the output buffer.
    /// @note  The data is gathered in a built-in queue, therefore
    ///        this function must be called repeatedly until the queue
    ///        is fully processed and returned. 
    /// @warning The queue will drop data if it is left to hit the limit.
    usize receive(std::span<char> out_buffer);
    
   private:
    /// @brief Worker thread execution function.
    void _worker_execute();

    /// @brief Internal implementation detail.
    void _process_samples();
    std::vector<f32> _samples_queue{};
    std::vector<char> _demodulated_data{};
    std::thread _worker{};
    std::atomic<usize> _head = 0; // Samples.
    std::atomic<usize> _tail = 0; // Samples.
    std::atomic<usize> _data_head = 0;
    std::atomic<usize> _data_tail = 0;
    std::atomic<bool> _terminate{};
};

}  // namespace mdm