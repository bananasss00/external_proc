#pragma once
#include "main.h"
#include "Process.h"

struct Process;
struct Ptr;

struct PeSection {
    ptr_t base;
    size_t size;

    PeSection(Process* process, ptr_t base, size_t size) noexcept : process_(process), base(base), size(size) {};

    auto is_valid() noexcept -> bool {
        return base && size;
    }

    auto find_pattern(const char* signature, int32_t add_offset = 0) noexcept -> Ptr;
    
private:
    Process* process_;
};