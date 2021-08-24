import ctypes
import os
import tempfile
import time
from ctypes import wintypes
from math import isclose
from typing import Optional

import sys
sys.path.append("..")  # use universal module from parent dir
from external_proc import *
import unittest

# TODO:
# find pattern for tutorial!
# assert tests
#  write.bytes(0x123456, b'\x90\x90', protected=True)
#  write.uint32(0x123456, 123456, protected=True)


# dll injector
def test_injector(process_name):
    print(':::TEST DLL INJECTION')
    with ExtProcess.ctx_open(process_name) as p:
        dll_name = 'testdll_x64.dll' if p.is_x64_process() else 'testdll_x32.dll'
        dll_path = os.path.abspath(dll_name)
        loadlib = get_proc_address('kernel32', 'LoadLibraryA', x64=p.is_x64_process())
        assert loadlib

        param = p.alloc()
        assert param

        p.write.str(param, dll_path)

        # can't be used if caller python_x32 and target_process_x64!!!
        with p.ctx_create_thread(loadlib, param, wait_thread=True) as th_id:
            pass


def test_shellcode(process_name, x64):
    print(':::TEST SHELLCODE INJECTION')

    def find_window(cls: Optional[str], title: Optional[str]):
        FindWindow = ctypes.WinDLL('user32').FindWindowA
        FindWindow.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        FindWindow.restype = ctypes.c_void_p
        return FindWindow(cls.encode('ascii') if cls else None, title.encode('ascii') if title else None)

    with ExtProcess.ctx_open(process_name) as tg:
        hwnd = find_window(None, 'Cheat Engine Tutorial v3.4')
        assert hwnd
        msg_box = get_proc_address('user32', 'MessageBoxA', x64=x64)
        assert msg_box
        alloc = tg.alloc(4096)
        assert alloc

        x64_shell = f'''
        xor r9, r9
        lea r8, @title
        lea rdx, @text
        lea rcx, {hwnd}
        mov rax, {msg_box}
        call rax
        '''

        x32_shell = f'''
        push 0
        push @title
        push @text
        push {hwnd}
        call {msg_box}
        '''

        shell = nasm(f'''
            sub esp, 0x28
        @start:
            cmp dword [@counter], 0x2
            je @exit
            mov eax, [@counter]
            inc eax
            mov [@counter], eax
            {x64_shell if x64 else x32_shell}
            jmp @start
        @exit:
            add esp,0x28
            ret
        @counter:
            dd 0
        @title:
            db 'title', 0
        @text:
            db 'text', 0
            ''', bits=64 if x64 else 32, origin=alloc)
        tg.write.bytes(alloc, shell)
        print(f'shellcode adr: {hex(alloc)}')

        # can't be used if caller python_x32 and target_process_x64!!!
        with tg.ctx_create_thread(alloc, wait_thread=True) as thread_id:
            pass


def test_memoperations(process_name):
    print(':::TEST MEMORY OPERATION')
    id = ExtProcess.get_process_id(process_name)
    assert id
    ids = ExtProcess.get_process_ids(process_name)
    assert len(ids)

    with ExtProcess.ctx_open(process_name) as p:
        is_x64_exe = 'x86_64' in process_name
        assert is_x64_exe == p.is_x64_process()  # can't be used if caller python_x32 and target_process_x64!!!
        assert p.get_exe_name().lower() == p.get_exe_name().lower()
        assert p.is_valid()

        module = p.get_module(process_name)  # can't be used if caller python_x32 and target_process_x64!!!
        assert module.is_valid()

        section = module.section(".text")  # can't be used if caller python_x32 and target_process_x64!!!
        assert section.is_valid()

        alloc = p.alloc(size=4096, address=0, fl_protect=PageFlags.PAGE_READWRITE)
        assert p.is_memory_readable(alloc)
        assert alloc

        old_protect = p.virtual_protect(alloc, 4096, PageFlags.PAGE_EXECUTE_READWRITE)
        assert old_protect == PageFlags.PAGE_READWRITE

        if is_x64_exe:
            # .text:10002EB68 EB 03  jmp short loc_10002EB6D
            # can't be used if caller python_x32 and target_process_x64!!!
            ptr = section.find_pattern("EB 03 0F 28 C1")  # 10002EB68
            assert ptr.get_address() == 0x10002EB68
            jmp_to = ptr.go_relative_ptr(instruction_offset=1, instruction_size=2, relative_adr_size=1)
            assert jmp_to.get_address() == 0x10002EB6D
            jmp_to = ptr.go_jmp_short_ptr()
            assert jmp_to.get_address() == 0x10002EB6D
            jmp_to = p.read.relative_offset_to_absolute(ptr.get_address(), 1, 2, 1)
            assert jmp_to == 0x10002EB6D
        else:
            # .text:004288CE EB 06  jmp short loc_4288D6
            ptr = section.find_pattern("EB 06 8B 45 D0")  # 004288CE
            assert ptr.get_address() == 0x4288CE
            jmp_to = ptr.go_relative_ptr(instruction_offset=1, instruction_size=2, relative_adr_size=1)
            assert jmp_to.get_address() == 0x4288D6
            jmp_to = ptr.go_jmp_short_ptr()
            assert jmp_to.get_address() == 0x4288D6
            jmp_to = p.read.relative_offset_to_absolute(ptr.get_address(), 1, 2, 1)
            assert jmp_to == 0x4288D6


        def test_writer_reader(writer, reader, value, size=0):
            writer(alloc, value)
            readed_value = reader(alloc, size) if size else reader(alloc)
            if type(value) is float:
                assert isclose(value, readed_value, rel_tol=0.00000001)
            else:
                assert readed_value == value

        test_writer_reader(p.write.list_bytes, p.read.list_bytes, [1, 2, 3, 4, 5, 6], 6)
        test_writer_reader(p.write.bytes, p.read.bytes, b'\x00\x01\x02\x03\x04\x05', 6)
        test_writer_reader(p.write.str, p.read.str, 'my string', 255)
        test_writer_reader(p.write.wstr, p.read.wstr, 'my wstring', 255)
        test_writer_reader(p.write.uint8, p.read.uint8, 255)
        test_writer_reader(p.write.uint16, p.read.uint16, 65535)
        test_writer_reader(p.write.uint32, p.read.uint32, 4294967295)
        test_writer_reader(p.write.uint64, p.read.uint64, 18_446_744_073_709_551_615)
        test_writer_reader(p.write.int8, p.read.int8, -128)
        test_writer_reader(p.write.int16, p.read.int16, -32768)
        test_writer_reader(p.write.int32, p.read.int32, -2147483647)
        test_writer_reader(p.write.int64, p.read.int64, -9_223_372_036_854_775_807)
        test_writer_reader(p.write.float, p.read.float, 3.40282e+038)
        test_writer_reader(p.write.double, p.read.double, 1.79769e+308)
        assert p.free(alloc)
    assert not p.is_valid()


def test_ptrs(process_name):
    print(':::TEST PTRS')
    with ExtProcess.ctx_open(process_name) as proc:
        def test(ptr_type, value, size=0):
            p = proc.make_ptr(alloc, ptr_type)
            p.set_value(value)
            assert p.is_memory_readable()
            readed_value = p.get_value(value_size=0 if not size else size)
            if type(value) is float:
                assert isclose(value, readed_value, rel_tol=0.0000001)
            else:
                assert readed_value == value

        alloc = proc.alloc()
        assert alloc
        assert proc.is_memory_readable(alloc)
        test(PtrType.ListBytes, [1,2,3,4,5,6], 6)
        test(PtrType.Bytes, b'privet\x99', 7)
        test(PtrType.Str, 'привет', 255)
        test(PtrType.Wstr, 'привет', 255)
        test(PtrType.Uint8, 255)
        test(PtrType.Uint16, 65535)
        test(PtrType.Uint32, 4294967295)
        test(PtrType.Uint64, 18_446_744_073_709_551_615)
        test(PtrType.Int8, -128)
        test(PtrType.Int16, -32768)
        test(PtrType.Int32, -2147483647)
        test(PtrType.Int64, -9_223_372_036_854_775_807)
        test(PtrType.Float, 3.40282e+038)
        test(PtrType.Double, 1.79769e+308)


# tests
os.system('chcp 65001')
for p_name in ['Tutorial-x86_64.exe', 'Tutorial-i386.exe']:
    print(f'::: ===> {p_name}')
    os.system(f'start CE_Tutorial/{p_name}')
    time.sleep(0.1)
    try:
        test_memoperations(p_name)
        test_injector(p_name)
        test_shellcode(p_name, 'x86_64' in p_name)
        test_ptrs(p_name)
    except PyException as e:
        print(f'\033[91m Exception: {e} \033[0m')
        # Exception: Access from x32 process to x64 has limited funcionality. Use python x64 version!
    os.system(f'taskkill /IM "{p_name}" /F')
    print('\n')
    # input('Press ENTER...')






