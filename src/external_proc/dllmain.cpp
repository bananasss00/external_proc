#include "main.h"
#include "Exception.h"
#include "Process.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

static string MODULE_PATH; 

DWORD get_last_error()
{
    return GetLastError();
}

class ProcessPublic : public Process { // helper type for exposing protected functions
public:
    using Process::process_handle_; // inherited with different access modifier
};

#ifndef _TEST
// _exthack имя нашего модуля
PYBIND11_MODULE(_external_proc, m) {
    // exceptions
    // make a new custom exception and use it as a translation target
    static py::exception<PyException> ex0(m, "PyException");
    py::register_exception_translator([](std::exception_ptr p) {
        try {
            if (p) std::rethrow_exception(p);
        } catch (const PyException &e) {
            // Set MyException as the active python error
            ex0(e.what());
        }
    });

    static py::exception<RequiredValueSizeArgument> ex(m, "RequiredValueSizeArgument");
    py::register_exception_translator([](std::exception_ptr p) {
        try {
            if (p) std::rethrow_exception(p);
        } catch (const RequiredValueSizeArgument &e) {
            // Set MyException as the active python error
            ex(e.what());
        }
    });

    static py::exception<UnknownPtrType> ex2(m, "UnknownPtrType");
    py::register_exception_translator([](std::exception_ptr p) {
        try {
            if (p) std::rethrow_exception(p);
        } catch (const UnknownPtrType &e) {
            // Set MyException as the active python error
            ex2(e.what());
        }
    });

    // structs
    py::enum_<PtrType>(m, "PtrType")
        .value("ListBytes", PtrType::ListBytes)
        .value("Bytes", PtrType::Bytes)
        .value("Str", PtrType::Str)
        .value("Wstr", PtrType::Wstr)
        .value("Uint8", PtrType::Uint8)
        .value("Uint16", PtrType::Uint16)
        .value("Uint32", PtrType::Uint32)
        .value("Uint64", PtrType::Uint64)
        .value("Int8", PtrType::Int8)
        .value("Int16", PtrType::Int16)
        .value("Int32", PtrType::Int32)
        .value("Int64", PtrType::Int64)
        .value("Float", PtrType::Float)
        .value("Double", PtrType::Double)
        .value("Invalid", PtrType::Invalid)
        .export_values();

    py::class_<Ptr>(m, "Ptr")
        .def(py::init<Process*, ptr_t, PtrType>(), "", py::arg("process"), py::arg("ptr"), py::arg("type"))
        .def("is_valid", &Ptr::is_valid)
        .def("is_memory_readable", &Ptr::is_memory_readable)
        .def("set_ptr_type", &Ptr::set_ptr_type)
        .def("get_value", &Ptr::get_value, "", py::arg("value_size") = 0)
        .def("set_value", &Ptr::set_value)
        .def("get_address", &Ptr::get_address)
        .def("go_ptr", &Ptr::go_ptr, "", py::arg("add_offset") = 0)
        .def("go_relative_ptr", &Ptr::go_relative_ptr, "", py::arg("instruction_offset"), py::arg("instruction_size"), py::arg("relative_adr_size"), py::arg("add_offset") = 0)
        .def("go_call_ptr", &Ptr::go_call_ptr, "", py::arg("add_offset") = 0)
        .def("go_jmp_ptr", &Ptr::go_jmp_ptr, "", py::arg("add_offset") = 0)
        .def("go_jmp_short_ptr", &Ptr::go_jmp_short_ptr, "", py::arg("add_offset") = 0);

    py::class_<PeSection>(m, "PeSection")
        // .def(py::init<>())
        // .def(py::init<Process*, ptr_t, size_t>())
        .def_readonly("base", &PeSection::base)
        .def_readonly("size", &PeSection::size)
        .def("is_valid", &PeSection::is_valid)
        .def("find_pattern", &PeSection::find_pattern, "", py::arg("signature"), py::arg("add_offset") = 0);

    py::class_<ProcessModule>(m, "ProcessModule")
        // .def(py::init<>())
        // .def(py::init<Process*, ptr_t, size_t>())
        .def_readonly("base", &ProcessModule::base)
        .def_readonly("size", &ProcessModule::size)
        .def("is_valid", &ProcessModule::is_valid)
        .def("find_pattern", &ProcessModule::find_pattern, "", py::arg("signature"), py::arg("add_offset") = 0)
        .def("section", &ProcessModule::section);

    py::class_<Writer>(m, "Writer")
        .def(py::init<uint64_t>())
        .def("list_bytes", &Writer::list_bytes)
        .def("bytes", &Writer::bytes)
        .def("str", &Writer::str)
        .def("wstr", &Writer::wstr)
        .def("uint8", &Writer::uint8)
        .def("uint16", &Writer::uint16)
        .def("uint32", &Writer::uint32)
        .def("uint64", &Writer::uint64)
        .def("int8", &Writer::int8)
        .def("int16", &Writer::int16)
        .def("int32", &Writer::int32)
        .def("int64", &Writer::int64)
        .def("float", &Writer::flt)
        .def("double", &Writer::dbl);

    py::class_<Reader>(m, "Reader")
        .def(py::init<uint64_t>())
        .def("relative_offset_to_absolute", &Reader::relative_offset_to_absolute)
        .def("list_bytes", &Reader::list_bytes)
        .def("bytes", &Reader::bytes)
        .def("str", &Reader::str)
        .def("wstr", &Reader::wstr)
        .def("uint8", &Reader::uint8)
        .def("uint16", &Reader::uint16)
        .def("uint32", &Reader::uint32)
        .def("uint64", &Reader::uint64)
        .def("int8", &Reader::int8)
        .def("int16", &Reader::int16)
        .def("int32", &Reader::int32)
        .def("int64", &Reader::int64)
        .def("float", &Reader::flt)
        .def("double", &Reader::dbl);

    py::class_<Process>(m, "Process")
        .def(py::init<DWORD>())
        .def(py::init<string>())
        .def_readonly("read", &Process::read)
        .def_readonly("write", &Process::write)
        .def_readonly("_process_handle", &ProcessPublic::process_handle_) // expose protected methods via the publicist
        .def("is_valid", &Process::is_valid)
        .def("close", &Process::close)
        .def_static("get_process_id", &Process::get_process_id)
        .def_static("get_process_ids", &Process::get_process_ids)
        .def("open_process", &Process::open_process)
        .def("close_handle", &Process::close_handle)
        .def("get_module", &Process::get_module, "", py::arg("name") = "")
        .def("get_exe_name", &Process::get_exe_name)
        .def("alloc", &Process::alloc, "", py::arg("size") = 4096, py::arg("address") = 0, py::arg("fl_protect") = PAGE_EXECUTE_READWRITE)
        .def("free", &Process::free)
        .def("virtual_protect", &Process::virtual_protect)
        .def("create_thread", &Process::create_thread, "", py::arg("address"), py::arg("parameter") = 0)
        .def("wait_for_single_object", &Process::wait_for_single_object)
        .def("get_pe_section", &Process::get_pe_section)
        .def("is_x64_process", &Process::is_x64_process)
        .def("find_pattern", &Process::find_pattern, "", py::arg("scan_start"), py::arg("scan_end"), py::arg("signature"), py::arg("add_offset") = 0)
        .def("make_ptr", &Process::make_ptr, "", py::arg("address"), py::arg("type"), py::arg("value_size") = 0)
        .def("is_memory_readable", &Process::is_memory_readable);

    // functions
    m.def("get_last_error", &get_last_error);
    
    // global vars
    m.attr("MODULE_PATH") = MODULE_PATH;
    m.attr("PAGE_EXECUTE") = PAGE_EXECUTE;
    m.attr("PAGE_EXECUTE_READ") = PAGE_EXECUTE_READ;
    m.attr("PAGE_EXECUTE_READWRITE") = PAGE_EXECUTE_READWRITE;
    m.attr("PAGE_READWRITE") = PAGE_READWRITE;
    
#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
};
#endif

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDll);
        // CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);

        char path[MAX_PATH] {0};
        if (GetModuleFileName((HMODULE)hinstDll, path, sizeof(path)) == 0)
        {
            int ret = GetLastError();
            fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
            // Return or however you want to handle an error.
        }
        MODULE_PATH = filesystem::path(path).parent_path().u8string();
	}
	return TRUE;
}

void main() {
    // exe mode tests
}