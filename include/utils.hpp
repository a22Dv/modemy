#pragma once

#include <string_view>
#include <stdexcept>

namespace mdy {

using err_msg = std::string_view;
inline void require(bool condition, err_msg message) {
    if (!condition) {
        throw std::runtime_error(message.data());
    }
}

}