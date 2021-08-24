#pragma once
#include "main.h"

struct Reader {
    Reader() noexcept {}

    Reader(whandle_t process_handle) noexcept {
        process_handle_ = process_handle;
    }

    auto relative_offset_to_absolute(ptr_t instruction_address, size_t relative_adr_offset, size_t instruction_size, size_t relative_adr_size) noexcept -> ptr_t {
        vector<uint8_t> bytes = list_bytes(instruction_address, instruction_size);
        if (!bytes.size()) return 0;

        int64_t relative;
        switch(relative_adr_size) {
            case 1:
                relative = *(int8_t*)(bytes.data() + relative_adr_offset);
                break;
            case 2:
                relative = *(int16_t*)(bytes.data() + relative_adr_offset);
                break;
            case 4:
                relative = *(int32_t*)(bytes.data() + relative_adr_offset);
                break;
            case 8:
                relative = *(int64_t*)(bytes.data() + relative_adr_offset);
                break;
            default:
                return 0; // TODO: raise Python exception! -> throw runtime_error("not supported 'relative_adr_size'");
        }
        return instruction_address + relative + instruction_size;
    }

    auto list_bytes(ptr_t address, size_t size) noexcept -> vector<uint8_t> {
        vector<uint8_t> bytes(size);
        if (!read(address, bytes.data(), size))
            return vector<uint8_t>();
        return bytes;
    }

    auto bytes(ptr_t address, size_t size) noexcept -> py::bytes {
        string bytes;
        bytes.reserve(size);
        bytes.resize(size);
        if (!read(address, bytes.data(), size))
            return py::bytes("");
        return py::bytes(bytes);
    }

    auto str(ptr_t address, size_t max_size) noexcept -> string {
        vector<uint8_t> bytes = list_bytes(address, max_size);
        if (!bytes.size()) return string();
        return string((char*)bytes.data());
    }

    auto wstr(ptr_t address, size_t max_size) noexcept -> wstring {
        vector<uint8_t> bytes = list_bytes(address, max_size);
        if (!bytes.size()) return wstring();
        return wstring((wchar_t*)bytes.data());
    }

    auto uint8(ptr_t address) noexcept -> uint8_t {
        return read<uint8_t>(address);
    }

    auto uint16(ptr_t address) noexcept -> uint16_t {
        return read<uint16_t>(address);
    }

    auto uint32(ptr_t address) noexcept -> uint32_t {
        return read<uint32_t>(address);
    }

    auto uint64(ptr_t address) noexcept -> uint64_t {
        return read<uint64_t>(address);
    }
    
    auto int8(ptr_t address) noexcept -> int8_t {
        return read<int8_t>(address);
    }

    auto int16(ptr_t address) noexcept -> int16_t {
        return read<int16_t>(address);
    }

    auto int32(ptr_t address) noexcept -> int32_t {
        return read<int32_t>(address);
    }

    auto int64(ptr_t address) noexcept -> int64_t {
        return read<int64_t>(address);
    }

    auto flt(ptr_t address) noexcept -> float {
        return read<float>(address);
    }

    auto dbl(ptr_t address) noexcept -> double {
        return read<double>(address);
    }

private:
    whandle_t process_handle_;

    auto read(ptr_t address, void* buff, size_t size) noexcept -> bool {
        SIZE_T out = NULL;
        BOOL result = ReadProcessMemory((HANDLE)process_handle_, (LPCVOID)address, (LPVOID)buff, (SIZE_T)size, &out);
        return out && result == TRUE;
    }

    template<typename T>
    auto read(ptr_t address, const T& t_default = T()) noexcept -> T {
	    T ret;
	    if( !read(address, &ret, sizeof(T))) {
		    return t_default;
	    }
	    return ret;
    }
};