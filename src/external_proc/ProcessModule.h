#pragma once
#include "main.h"
#include "Process.h"

struct Process;
struct PeSection;

struct Ptr;

struct ProcessModule {
    ptr_t base;
    size_t size;
    // ProcessModule() {}

    ProcessModule(Process* process, ptr_t base, size_t size) noexcept : process_(process), base(base), size(size) {};

    auto is_valid() noexcept -> bool {
        return base && size;
    }

    auto find_pattern(const char* signature, int32_t add_offset = 0) noexcept -> Ptr;

    auto section(const char* name) noexcept -> PeSection;
    
private:
    Process* process_;
};