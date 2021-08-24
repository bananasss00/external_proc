#include "ProcessModule.h"

auto ProcessModule::find_pattern(const char* signature, int32_t add_offset) noexcept -> Ptr {
    return process_->find_pattern(base, base + size, signature, add_offset);
}

auto ProcessModule::section(const char* name) noexcept -> PeSection {
    return process_->get_pe_section(base, name);
}

