#include "CallerThreadExecutor.h"




namespace exec {
CallerThreadExecutor::CallerThreadExecutor(const common::Options&) noexcept {
}

std::size_t CallerThreadExecutor::capacity() noexcept {
    return 0;
}

} // namespace exec
