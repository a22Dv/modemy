#include <algorithm>
#include <atomic>

#include "demodulator.hpp"

namespace mdm {

constexpr usize _data_queue_size = 512;
constexpr usize _samples_queue_size = 2048;

Demodulator::Demodulator() {
    _samples_queue = std::vector(_samples_queue_size, 0.0f);
    _demodulated_data = std::vector(_data_queue_size, '\0');
    _worker = std::thread{[this] {
        return _worker_execute();
    }};
}

Demodulator::~Demodulator() {
    _terminate.store(true, std::memory_order_release);
    if (_worker.joinable()) {
        _worker.join();
    }
}

void Demodulator::push_samples(std::span<const f32> samples) {
    usize samples_proc = 0;
    const usize samples_count = samples.size();
    while (samples_proc < samples_count) {
        const usize head = _head.load(std::memory_order_relaxed);
        const usize tail = _tail.load(std::memory_order_acquire);
        if ((head + 1) % _samples_queue_size == tail) {
            _head.wait(head, std::memory_order_acquire);
            continue;
        }
        const usize available = (_samples_queue_size * (head <= tail) - tail + head) - 1;
        const usize read_count =
            std::min({available, _samples_queue_size - head, samples_count - samples_proc});
        std::copy_n(samples.begin() + samples_proc, read_count, _samples_queue.begin() + head);
        _head.store((head + read_count) % _samples_queue_size, std::memory_order_release);
        samples_proc += read_count;
    }
}

usize Demodulator::receive(std::span<char> out_buffer) {
    const usize head = _data_head.load(std::memory_order_acquire);
    usize tail = _data_tail.load(std::memory_order_release);
    const usize complete_bytes = std::min(out_buffer.size(), _data_queue_size * (tail > head) + head - tail);

    usize bytes_received = 0;
    while (bytes_received < complete_bytes) {
        const usize read_count = std::min(complete_bytes - bytes_received, _data_queue_size - tail);
        std::copy_n(_demodulated_data.begin() + tail, read_count, out_buffer.begin() + bytes_received);
        tail = (tail + read_count) % _data_queue_size;
        bytes_received += read_count;
    }
    _data_tail.store(tail, std::memory_order_release);
    return bytes_received;
}

void Demodulator::_worker_execute() {
    
}

void Demodulator::_process_samples() {
}

}  // namespace mdm