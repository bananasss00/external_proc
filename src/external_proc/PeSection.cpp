#include "PeSection.h"

auto PeSection::find_pattern(const char* signature, int32_t add_offset) noexcept -> Ptr {
    return process_->find_pattern(base, base + size, signature, add_offset);
}

