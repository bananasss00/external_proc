#pragma once
#include "Exception.h"
#include "main.h"
#include "Process.h"

struct Process;

enum PtrType {
    ListBytes = 0,
    Bytes,
    Str,
    Wstr,
    Uint8,
    Uint16,
    Uint32,
    Uint64,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
    Invalid
};

struct Ptr {
    Ptr(Process* process, ptr_t ptr, PtrType type) noexcept : process_(process), ptr_(ptr), type_(type) {
    };

    auto is_valid() noexcept -> bool {
        return ptr_;
    }

    auto is_memory_readable() noexcept -> bool;

    auto set_ptr_type(PtrType type) -> Ptr {
        type_ = type;
        return Ptr(process_, ptr_, type_);
    }

    auto get_value(size_t value_size = 0) -> py::object;

    auto set_value(py::object value) -> bool;

    auto get_address() noexcept -> ptr_t {
        return ptr_;
    }

    auto go_ptr(int32_t add_offset = 0) noexcept -> Ptr;

    auto go_relative_ptr(int32_t instruction_offset, int32_t instruction_size, size_t relative_adr_size, int32_t add_offset = 0) noexcept -> Ptr;

    auto go_call_ptr(int32_t add_offset = 0) noexcept -> Ptr;

    auto go_jmp_ptr(int32_t add_offset = 0) noexcept -> Ptr;

    auto go_jmp_short_ptr(int32_t add_offset = 0) noexcept -> Ptr;

private:
    Process* process_;
    ptr_t ptr_;
    PtrType type_;
};