#pragma once

#include "./context.h"

#include <functional>

namespace coro {

using Function = std::function<void (const coro::Context&)>;

}