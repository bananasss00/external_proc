#include "Process.h"

Process::Process(pid_t process_id) {
    if (!process_id)
        return;
    open_process(process_id);
    read = Reader(process_handle_);
    write = Writer(process_handle_);
}

Process::Process(string process_name) {
    pid_t process_id = get_process_id(process_name.c_str());
    if (!process_id)
        return;
    open_process(process_id);
    read = Reader(process_handle_);
    write = Writer(process_handle_);
}

Process::~Process() noexcept {
    // cout << "destructor _process called" << endl;
    if (is_valid()) close_handle(process_handle_);
}

auto Process::is_valid() noexcept -> bool {
    DWORD exit_code;
    BOOL api_result = GetExitCodeProcess((HANDLE)process_handle_, &exit_code);
    return process_handle_ && api_result == TRUE && exit_code == STILL_ACTIVE;
}

auto Process::close() noexcept -> void {
    close_handle(process_handle_);
}

// OPEN PROCESS //
auto Process::get_process_id(const char* process_name) noexcept -> pid_t {
    vector<pid_t> ids = get_process_ids(process_name);
    if (ids.size() == 0) return 0;
    return ids.at(0);
}

auto Process::get_process_ids(const char* process_name) noexcept -> vector<pid_t> {
    string name = to_lower(process_name);
    vector<pid_t> ids;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(entry);
    do
    {
        string cur_name = to_lower(entry.szExeFile);
        if (cur_name == name)
        {
            pid_t pid = entry.th32ProcessID;
            ids.push_back(pid);
        }
    }
    while (Process32Next(snapshot, &entry));
    CloseHandle(snapshot);
    return ids;
}

auto Process::open_process(pid_t process_id) -> void {
    // HANDLE hToken; 
    // TOKEN_PRIVILEGES tp; 
    // HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId() ); 

    // tp.PrivilegeCount = 1; 
    // LookupPrivilegeValueA( NULL, "SeDebugPrivilege", &tp.Privileges[0].Luid ); 
    // tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
    // OpenProcessToken( hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken ); 

    // AdjustTokenPrivileges( hToken, FALSE, &tp, NULL, NULL, NULL ); 
    // CloseHandle( hToken );
    // CloseHandle( hProcess );

    process_id_ = process_id;
    process_handle_ = (uint64_t)OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);

#if !defined(_M_X64) && !defined(__amd64__)
    BOOL is_wow64[2];
    HANDLE current_handle = GetCurrentProcess();
    if (TRUE == IsWow64Process((HANDLE)process_handle_, &is_wow64[0])
        && TRUE == IsWow64Process(current_handle, &is_wow64[1])) {
        if (is_wow64[0] == FALSE && is_wow64[1] == TRUE) {
            // cout << "Access from x32 process to x64 has limited funcionality. Use python x64 version!\n";
            throw PyException("Access from x32 process to x64 has limited funcionality. Use python x64 version!");
        }
    }
#endif
}

auto Process::close_handle(whandle_t process) noexcept -> void {
    CloseHandle((HANDLE)process);
}

auto Process::get_module(string name/* = ""*/) noexcept -> ProcessModule {
    if (name.empty()) {
        name = get_exe_name();
    }
    name = to_lower(name);

    /* BOTH VARIANTS NOT WORK FROM x32 PROCESS FOR x64 PROCESS */
 #if 0
    MODULEINFO minfo;
    HMODULE hmodules[1024];
    DWORD needed;
    char mod_name[MAX_PATH];
    if (EnumProcessModulesEx((HANDLE)process_handle_, hmodules, sizeof(hmodules), &needed, LIST_MODULES_ALL)) {
        if (needed > sizeof(hmodules)) { // small buffer for modules
            return ProcessModule(this, 0, 0);
        }
        for (DWORD i = 0; i < (needed / sizeof(HMODULE)); i++) {
            if (!GetModuleBaseNameA((HANDLE)process_handle_, hmodules[i], mod_name, sizeof(mod_name))) {
                continue;
            }
            if (GetModuleInformation((HANDLE)process_handle_, hmodules[i], &minfo, sizeof(minfo))) {
                string module = to_lower(mod_name);
                if (module == name) {
                    return ProcessModule(this, (ptr_t)hmodules[i], minfo.SizeOfImage);
                }
            }
        }
    }
    else printf("Can't enumerate modules\n");
#else
    // 299 - ERROR_PARTIAL_COPY. x32 process to x64 process 
    auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id_);
    if  (snapshot == INVALID_HANDLE_VALUE) {
        return ProcessModule(this, 0, 0);
    }
    MODULEENTRY32 entry;
    entry.dwSize = sizeof(entry);
    do {
        string module = to_lower(entry.szModule);
        if (module == name) {
            CloseHandle(snapshot);
            return ProcessModule(this, (ptr_t)entry.hModule, entry.modBaseSize);
        }
    }
    while (Module32Next(snapshot, &entry));
#endif

    return ProcessModule(this, 0, 0);
}

auto Process::get_exe_name() noexcept -> string {
    if (exe_name_.empty()) {
        char name[MAX_PATH] {0}; 
        GetModuleFileNameExA((HANDLE)process_handle_, NULL, name, MAX_PATH);
        exe_name_ = filesystem::path(name).filename().u8string();
    }
    return exe_name_;
}

// PROCESS OPERATIONS //
auto Process::alloc(size_t size, ptr_t address, DWORD fl_protect) noexcept -> ptr_t {
    ptr_t mem = (ptr_t)VirtualAllocEx((HANDLE)process_handle_, (LPVOID)address, size, MEM_RESERVE | MEM_COMMIT, fl_protect);
    if (address && !mem) { // try find near possible alloc address. @CheatEngine -> autoassembler.pas:lastChanceAllocPrefered(...)
        SYSTEM_INFO systeminfo;
        GetSystemInfo(&systeminfo);
        auto starttime = GetTickCount64();
        size_t distance = 0;
        size_t count = 0;

        if (address % systeminfo.dwAllocationGranularity > 0) {
            address = address - (address % systeminfo.dwAllocationGranularity);
        }

        while (!mem && (count < 10 || (GetTickCount64() < starttime + 10000)) && (distance < 0x80000000)) {
            mem = (ptr_t)VirtualAllocEx((HANDLE)process_handle_, (LPVOID)(address + distance), size, MEM_RESERVE | MEM_COMMIT, fl_protect);
            if (!mem && distance > 0) {
                mem = (ptr_t)VirtualAllocEx((HANDLE)process_handle_, (LPVOID)(address - distance), size, MEM_RESERVE | MEM_COMMIT, fl_protect);
            }
            if (!mem) {
                distance += systeminfo.dwAllocationGranularity;
            }
            count++;
        }
    } 
    return mem;
}

auto Process::free(ptr_t address) noexcept -> bool {
    return VirtualFreeEx((HANDLE)process_handle_, (LPVOID)address, (SIZE_T)0, MEM_RELEASE) == TRUE;
}

auto Process::virtual_protect(ptr_t address, size_t size, int flags) noexcept -> DWORD {
    DWORD old_protect = 0;
    VirtualProtectEx((HANDLE)process_handle_, (LPVOID)address, size, flags, &old_protect);
    return old_protect;
}

auto Process::create_thread(ptr_t address, ptr_t parameter) noexcept -> whandle_t {
    return (whandle_t)CreateRemoteThread((HANDLE)process_handle_, 0, 0, (LPTHREAD_START_ROUTINE)address, (LPVOID)parameter, 0, 0);
}

auto Process::wait_for_single_object(whandle_t handle) noexcept -> void {
    WaitForSingleObject((HANDLE)handle, INFINITE);
}

auto Process::get_pe_section(ptr_t module_ptr, const char* section_name) noexcept -> PeSection {
    vector<uint8_t> bytes = read.list_bytes(module_ptr, 0x1000);
    if (!bytes.size())
        return PeSection(this, 0, 0);
    PIMAGE_NT_HEADERS nt_header = get_nt_header(bytes);
    if (!nt_header)
        return PeSection(this, 0, 0);
    
    uint64_t section_location = (uint64_t)nt_header + sizeof(uint32_t) + (sizeof(IMAGE_FILE_HEADER)) + nt_header->FileHeader.SizeOfOptionalHeader;
    uint64_t section_size = sizeof(IMAGE_SECTION_HEADER);
    
    for (int i = 0; i < nt_header->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER section_header = (PIMAGE_SECTION_HEADER)section_location;
        if (_stricmp((LPCSTR)section_header->Name, section_name) == 0) {
            auto offset = (ptr_t)module_ptr + section_header->VirtualAddress;
            size_t len = section_header->SizeOfRawData;
            return PeSection(this, offset, len);
        }
        section_location += section_size;
    }
    
    return PeSection(this, 0, 0);
}

auto Process::is_x64_process() noexcept -> bool {
    if (!get<0>(is_x64_)) {
        auto exe_name = get_exe_name();
        ProcessModule module = get_module(exe_name);
        vector<uint8_t> bytes = read.list_bytes(module.base, 0x1000);
        if (!bytes.size()) {
            is_x64_ = {true, false};
            return false;
        }
        PIMAGE_NT_HEADERS nt_header = get_nt_header(bytes);
        if (!nt_header) {
            is_x64_ = {true, false};
            return false;
        }

        is_x64_ = {true, nt_header->OptionalHeader.Magic == 0x020B};
    }
    return get<1>(is_x64_);
}

auto Process::find_pattern(ptr_t scan_start, ptr_t scan_end, const char* signature, int32_t add_offset) noexcept -> Ptr {
    size_t chunk_size = size_t(scan_end - scan_start);
    vector<uint8_t> bytes = read.list_bytes(scan_start, chunk_size);
    
    if (!bytes.size())
        return Ptr(this, 0, PtrType::Invalid);
    
    vector<int> patternBytes = pattern_to_bytes(signature);
    uint8_t*    scanBytes = bytes.data();
    size_t      s = patternBytes.size();
    int*        d = patternBytes.data();

    ptr_t scan_max = scan_end - scan_start - s;
    for(ptr_t i = 0ul; i < scan_max; ++i) {
        bool found = true;
        for(ptr_t j = 0ul; j < s; ++j) {
            if(scanBytes[i + j] != d[j] && d[j] != -1) {
                found = false;
                break;
            }
        }
        if(found) {
            return Ptr(this, scan_start + i + add_offset, is_x64_process() ? PtrType::Uint64 : PtrType::Uint32);
        }
    }

    return Ptr(this, 0, PtrType::Invalid);
}

auto Process::make_ptr(ptr_t address, PtrType type, size_t value_size) noexcept -> Ptr {
    return Ptr(this, address, type);
}

auto Process::is_memory_readable(ptr_t address) noexcept -> bool {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx((HANDLE)process_handle_, (LPCVOID)address, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
        return false;

    if (mbi.State != MEM_COMMIT)
        return false;

    if (mbi.Protect == PAGE_NOACCESS || mbi.Protect == PAGE_EXECUTE)
        return false;

    return true;
}
