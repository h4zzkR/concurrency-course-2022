#pragma once

#include <wheels/support/function.hpp>

namespace tp {

// Intrusive tasks?
using Task = wheels::UniqueFunction<void()>;

}  // namespace tp
