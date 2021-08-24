#pragma once
#include "main.h"
#include "Writer.h"
#include "Reader.h"
#include "PeSection.h"
#include "ProcessModule.h"
#include "Ptr.h"

struct ProcessModule;
struct PeSection;
struct Ptr;
enum PtrType;

struct Process {
    Reader read;
    Writer write;

    Process(pid_t process_id);

    Process(string process_name);

    ~Process() noexcept;

    auto is_valid() noexcept -> bool;

    auto close() noexcept -> void;

    static auto get_process_id(const char* process_name) noexcept -> pid_t;

    static auto get_process_ids(const char* process_name) noexcept -> vector<pid_t>;

    auto open_process(pid_t process_id) -> void;

    auto close_handle(whandle_t process) noexcept -> void;

    auto get_module(string name = "") noexcept -> ProcessModule;

    auto get_exe_name() noexcept -> string;

    auto alloc(size_t size = 4096, ptr_t address = 0, DWORD fl_protect = PAGE_EXECUTE_READWRITE) noexcept -> ptr_t;

    auto free(ptr_t address) noexcept -> bool;

    auto virtual_protect(ptr_t address, size_t size, int flags) noexcept -> DWORD;

    auto create_thread(ptr_t address, ptr_t parameter = 0) noexcept -> whandle_t;

    auto wait_for_single_object(whandle_t handle) noexcept -> void;

    auto get_pe_section(ptr_t module_ptr, const char* section_name) noexcept -> PeSection;

    auto is_x64_process() noexcept -> bool;

    auto find_pattern(ptr_t scan_start, ptr_t scan_end, const char* signature, int32_t add_offset = 0) noexcept -> Ptr;

    auto make_ptr(ptr_t address, PtrType type, size_t value_size = 0) noexcept -> Ptr;
    
    auto is_memory_readable(ptr_t address) noexcept -> bool;

protected:
    pid_t process_id_;
    whandle_t process_handle_;
    string exe_name_;
    tuple<bool, bool> is_x64_;

    static auto to_lower(string str) noexcept -> string {
        transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return tolower(c); });
        return str;
    }

    static auto pattern_to_bytes(const char* pattern) noexcept -> vector<int> {
        auto bytes = vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        for(auto current = start; current < end; ++current) {
            if(*current == '?') {
                ++current;
                if(*current == '?')
                    ++current;
                bytes.push_back(-1);
            } else {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
    }

    static auto get_nt_header(const vector<uint8_t>& bytes) noexcept -> PIMAGE_NT_HEADERS {
        PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)bytes.data();

        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
            return nullptr;

        PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)((uint64_t)bytes.data() + dos_header->e_lfanew);
        if (nt_header->Signature != IMAGE_NT_SIGNATURE)
            return nullptr;

        return nt_header;
    }
};