#pragma once
#include "main.h"

struct Writer {
    Writer() noexcept {}
  
    Writer(whandle_t process_handle) noexcept {
        process_handle_ = process_handle;
    }

    auto list_bytes(ptr_t address, vector<uint8_t> bytes) noexcept -> bool {
        return write(address, bytes.data(), bytes.size());
    }

    auto bytes(ptr_t address, string bytes) noexcept -> bool {
        return write(address, bytes.data(), bytes.size());
    }

    auto str(ptr_t address, string str) noexcept -> bool {
        return write(address, str.data(), str.size());
    }

    auto wstr(ptr_t address, wstring str) noexcept -> bool {
        return write(address, str.data(), str.size() * sizeof(wchar_t));
    }

    auto uint8(ptr_t address, uint8_t value) noexcept -> bool {
        return write(address, value);
    }

    auto uint16(ptr_t address, uint16_t value) noexcept -> bool {
        return write(address, value);
    }

    auto uint32(ptr_t address, uint32_t value) noexcept -> bool {
        return write(address, value);
    }

    auto uint64(ptr_t address, uint64_t value) noexcept -> bool {
        return write(address, value);
    }
    
    auto int8(ptr_t address, int8_t value) noexcept -> bool {
        return write(address, value);
    }

    auto int16(ptr_t address, int16_t value) noexcept -> bool {
        return write(address, value);
    }

    auto int32(ptr_t address, int32_t value) noexcept -> bool {
        return write(address, value);
    }

    auto int64(ptr_t address, int64_t value) noexcept -> bool {
        return write(address, value);
    }

    auto flt(ptr_t address, float value) noexcept -> bool {
        return write(address, value);
    }

    auto dbl(ptr_t address, double value) noexcept -> bool {
        return write(address, value);
    }



private:
    whandle_t process_handle_;

    auto write(ptr_t address, void* buff, size_t size) noexcept -> bool {
        SIZE_T out = NULL;
        BOOL result = WriteProcessMemory((HANDLE)process_handle_, (LPVOID)address, (LPCVOID)buff, (SIZE_T)size, &out);
        return out && result == TRUE;
    }

    template <typename T>
    auto write(ptr_t address, T value) noexcept -> bool { 
        return write(address, &value, sizeof(T));
    }
};