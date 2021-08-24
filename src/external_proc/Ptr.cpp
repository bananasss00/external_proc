#include "Ptr.h"

auto Ptr::is_memory_readable() noexcept -> bool {
    return process_->is_memory_readable(ptr_);
}

auto Ptr::get_value(size_t value_size/* = 0*/) -> py::object {
    switch (type_) {
        case ListBytes:
            if (!value_size) throw RequiredValueSizeArgument();
            return py::cast(process_->read.list_bytes(ptr_, value_size));
        case Bytes:
            if (!value_size) throw RequiredValueSizeArgument();
            return process_->read.bytes(ptr_, value_size);
        case Str:
            if (!value_size) throw RequiredValueSizeArgument();
            return py::cast(process_->read.str(ptr_, value_size));
        case Wstr:
            if (!value_size) throw RequiredValueSizeArgument();
            return py::cast(process_->read.wstr(ptr_, value_size));
        case Uint8:
            return py::cast(process_->read.uint8(ptr_));
        case Uint16:
            return py::cast(process_->read.uint16(ptr_));
        case Uint32:
            return py::cast(process_->read.uint32(ptr_));
        case Uint64:
            return py::cast(process_->read.uint64(ptr_));
        case Int8:
            return py::cast(process_->read.int8(ptr_));
        case Int16:
            return py::cast(process_->read.int16(ptr_));
        case Int32:
            return py::cast(process_->read.int32(ptr_));
        case Int64:
            return py::cast(process_->read.int64(ptr_));
        case Float:
            return py::cast(process_->read.flt(ptr_));
        case Double:
            return py::cast(process_->read.dbl(ptr_));
        case Invalid:
            break;
        default:
            throw UnknownPtrType();
    }
    return py::cast(nullptr);
}

auto Ptr::set_value(py::object value) -> bool {
    switch (type_) {
        case ListBytes:
            return process_->write.list_bytes(ptr_, value.cast<vector<uint8_t>>());
        case Bytes:
            return process_->write.bytes(ptr_, value.cast<string>());
        case Str:
            return process_->write.str(ptr_, value.cast<string>());
        case Wstr: 
            return process_->write.wstr(ptr_, value.cast<wstring>());
        case Uint8:
            return process_->write.uint8(ptr_, value.cast<uint8_t>());
        case Uint16:
            return process_->write.uint16(ptr_, value.cast<uint16_t>());
        case Uint32:
            return process_->write.uint32(ptr_, value.cast<uint32_t>());
        case Uint64:
            return process_->write.uint64(ptr_, value.cast<uint64_t>());
        case Int8:
            return process_->write.int8(ptr_, value.cast<int8_t>());
        case Int16:
            return process_->write.int16(ptr_, value.cast<int16_t>());
        case Int32:
            return process_->write.int32(ptr_, value.cast<int32_t>());
        case Int64:
            return process_->write.int64(ptr_, value.cast<int64_t>());
        case Float:
            return process_->write.flt(ptr_, value.cast<float>());
        case Double:
            return process_->write.dbl(ptr_, value.cast<double>());
        case Invalid:
            break;
        default:
            throw UnknownPtrType();
    }
    return false; 
}

auto Ptr::go_ptr(int32_t add_offset) noexcept -> Ptr {
    if (process_->is_x64_process()) {
        auto ptr = process_->read.uint64(ptr_);
        return Ptr(process_, ptr ? ptr + add_offset : 0, type_);
    } else {
        auto ptr = process_->read.uint32(ptr_);
        return Ptr(process_, ptr ? ptr + add_offset : 0, type_);
    }
}

auto Ptr::go_relative_ptr(int32_t instruction_offset, int32_t instruction_size, size_t relative_adr_size, int32_t add_offset) noexcept -> Ptr {
    ptr_t ptr = process_->read.relative_offset_to_absolute(ptr_, instruction_offset, instruction_size, relative_adr_size);
    return Ptr(process_, ptr ? ptr + add_offset : 0, type_);
}

auto Ptr::go_call_ptr(int32_t add_offset) noexcept -> Ptr {
    return go_relative_ptr(1, 5, 4, add_offset);
}

auto Ptr::go_jmp_ptr(int32_t add_offset) noexcept -> Ptr {
    return go_relative_ptr(1, 5, 4, add_offset);
}

auto Ptr::go_jmp_short_ptr(int32_t add_offset) noexcept -> Ptr {
    return go_relative_ptr(1, 2, 1, add_offset);
}

