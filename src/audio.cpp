#define MINIAUDIO_IMPLEMENTATION
#define NOMINMAX

#pragma warning(push, 1)
extern "C" {
#include <miniaudio.h>
}
#pragma warning(pop)

#include <algorithm>
#include <span>
#include <vector>

#include "audio.hpp"
#include "types.hpp"

namespace mdm {

void AudioDevice::read(std::span<f32> samples) {
    using mem = std::memory_order;

    const usize sample_count = samples.size();
    const usize buffer_size = _in_samples.size();

    usize samples_read = 0;
    while (samples_read < sample_count) {
        const usize w_idx = _in_head.load(mem::acquire);
        const usize r_idx = _in_tail.load(mem::relaxed);
        const usize dist = (buffer_size * (w_idx < r_idx)) - r_idx + w_idx;
        if (!dist) {
            _in_head.wait(w_idx, mem::acquire);
            continue;
        }
        const usize read_count = std::min({sample_count - samples_read, dist, buffer_size - r_idx});
        std::copy_n(_in_samples.begin() + r_idx, read_count, samples.begin() + samples_read);
        _in_tail.store((r_idx + read_count) % buffer_size, mem::release);
        samples_read += read_count;
    }
}

/// TODO: Implementation.
void AudioDevice::write(std::span<const f32> samples) {
}

}  // namespace mdm