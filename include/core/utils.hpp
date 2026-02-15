#pragma once

#include <stdexcept>
#include <string_view>

namespace mdm {

using err_str = std::string_view;
inline void require(bool condition, err_str str) {
    if (!condition) {
        throw std::runtime_error(str.data());
    }
}

}  // namespace mdm