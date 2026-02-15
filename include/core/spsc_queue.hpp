#pragma once
#pragma warning(push, 1)

#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <span>

#define MDM_ALIGNAS_HDI alignas(std::hardware_destructive_interference_size)

namespace mdm {

using usize = std::size_t;
using mem = std::memory_order;

template <usize _size, typename T>
class SPSCQueue {
   public:
    template <bool _blocking>
    usize push(std::span<const T> e) {
        const usize ecount = e.size();
        usize head = _head.load(mem::relaxed);
        usize tail = _tail.load(mem::acquire);
        usize eproc = 0;
        while (eproc < ecount) {
            if (head - tail != _size) {
                _cq[head & _mask] = e[eproc];
                ++head;
                ++eproc;
                continue;
            }
            if constexpr (_blocking) {
                _head.store(head, mem::release);
                _head.notify_one();
                _tail.wait(tail, mem::acquire);
                tail = _tail.load(mem::acquire);
                continue;
            } else {
                break;
            }
        }
        _head.store(head, mem::release);
        _head.notify_one();
        return eproc;
    }
    template <bool _blocking>
    usize pull(std::span<T> e) {
        const usize ecount = e.size();
        usize head = _head.load(mem::acquire);
        usize tail = _tail.load(mem::relaxed);
        usize eproc = 0;
        while (eproc < ecount) {
            if (head != tail) {
                e[eproc] = _cq[tail & _mask];
                ++tail;
                ++eproc;
                continue;
            }
            if constexpr (_blocking) {
                _tail.store(tail, mem::release);
                _tail.notify_one();
                _head.wait(head, mem::acquire);
                head = _head.load(mem::acquire);
                continue;
            } else {
                break;
            }
        }
        _tail.store(tail, mem::release);
        _tail.notify_one();
        return eproc;
    }
   private:
    static_assert(((_size - 1) & (_size)) == 0);

    std::array<T, _size> _cq{};
    MDM_ALIGNAS_HDI std::atomic<usize> _head = 0;
    MDM_ALIGNAS_HDI std::atomic<usize> _tail = 0;
    const usize _mask = _size - 1;
};

}  // namespace mdm

#pragma warning(pop)