import ctypes
import os
import subprocess
import tempfile

import external_proc._external_proc as core
from contextlib import contextmanager
from typing import Tuple, Union, List, Iterator, ContextManager, Type
import traceback


class PageFlags:
    PAGE_EXECUTE = core.PAGE_EXECUTE
    PAGE_EXECUTE_READ = core.PAGE_EXECUTE_READ
    PAGE_EXECUTE_READWRITE = core.PAGE_EXECUTE_READWRITE
    PAGE_READWRITE = core.PAGE_READWRITE


class Sys:
    MODULE_PATH = core.MODULE_PATH


class ExtProcess(core.Process):
    def __init__(self, process_id_or_name: Union[int, str]):
        super().__init__(process_id_or_name)

    @staticmethod
    def ctx_open(process_id_or_name: Union[int, str]) -> ContextManager['ExtProcess']:
        class Ctx(object):
            def __init__(self, process_id_or_name):
                self.obj = ExtProcess(process_id_or_name)
                if not self.obj._process_handle:
                    raise Exception(f"Can't open process! LastError: {core.get_last_error()}")

            def __enter__(self):
                return self.obj

            def __exit__(self, type, value, tb):
                self.obj.close()
                if type and traceback:
                    traceback.print_exception(type, value, tb)
                    # print(type, value, traceback)
                return True
        return Ctx(process_id_or_name)

    @staticmethod
    def ctx_open_all(process_name: str) -> ContextManager[List['ExtProcess']]:
        class Ctx(object):
            def __init__(self, process_name):
                procs = core.Process.get_process_ids(process_name)
                self.obj = [ExtProcess(p) for p in procs]
                if not self.obj._process_handle:
                    raise Exception(f"Can't open process! LastError: {core.get_last_error()}")

            def __enter__(self):
                return self.obj

            def __exit__(self, type, value, tb):
                for p in self.obj:
                    p.close()
                if type and traceback:
                    traceback.print_exception(type, value, tb)
                    # print(type, value)
                return True
        return Ctx(process_name)

    def ctx_create_thread(self, address: int, parameter: int = 0, wait_thread=False) -> int:
        _self = self

        class Ctx(object):
            def __init__(self):
                self.thread_id = None

            def __enter__(self):
                self.thread_id = _self.create_thread(address, parameter)
                if not self.thread_id:
                    raise Exception(f"Can't create thread! LastError: {core.get_last_error()}")
                if wait_thread:
                    _self.wait_for_single_object(self.thread_id)
                return self.thread_id

            def __exit__(self, type, value, tb):
                _self.close_handle(self.thread_id)
                if type and traceback:
                    traceback.print_exception(type, value, tb)
                    # print(type, value)
                return True
        return Ctx()


def get_proc_address(sys_module: str, func: str, x64: bool = True) -> int:
    fn = f'{Sys.MODULE_PATH}/get_proc_{"x64" if x64 else "x32"}.exe'
    with subprocess.Popen((fn, sys_module, func), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as process:
        out, err = process.communicate()
        return int(out.decode(), 16)


def gen_shellcode(base_address, ptr_size, *args, **kwargs):
    '''
    gen_shellcode(alloc, 4,
        '@start',
        b'\x81\x3D', 'counter', (0x5, 4), # cmp [counter], 0x5
        b'\x74', '>exit:1',               # je 06AD001D'
        b'\xA1', 'counter',               # mov eax,[counter]
        b'\x40',                          # inc eax
        b'\xA3', 'counter',               # mov [counter], eax
        ### message box ###
        b'\x6A\x00',                      # push 0
        b'\x68', 'title',                 # push 'title'
        b'\x68', 'text',                  # push 'text'
        b'\x68', tg_window,               # push hwnd
        b'\xB8', msg_box,                 # mov eax, msgbox_func
        b'\xFF\xD0',                      # call eax
        ### message box ###
        b'\xEB', '>start:1',             # jmp @start
        '@exit',
        b'\xC3',                          # ret
        title='This is message title', text='This is message text', counter=(0, 4)
    )
    :param base_address:
    :param ptr_size:
    :param args:
    :param kwargs:
    :return:
    '''
    goto = dict()
    labels = dict()
    refs = dict()
    res = bytes()
    pos = 0
    # parse references. push first in to byte array
    for k in kwargs.keys():
        v = kwargs[k]
        if type(v) is str:
            val = v.encode() + b'\x00'
            res = res + val
            refs[k] = base_address + pos
            pos = pos + len(val)
        elif type(v) is bytes:
            res = res + v
            refs[k] = base_address + pos
            pos = pos + len(v)
        elif type(v) is int:
            val = v.to_bytes(ptr_size, 'little')
            res = res + val
            refs[k] = base_address + pos
            pos = pos + len(val)
        elif type(v) is tuple:
            val = v[0].to_bytes(v[1], 'little')
            res = res + val
            refs[k] = base_address + pos
            pos = pos + len(val)

    executable = base_address + pos
    for a in args:
        if type(a) is str:
            if a.startswith('@'):
                labels[a[1:]] = pos
            elif a.startswith('>'):
                label, size = a[1:].split(':')
                size = int(size)
                goto[label] = (pos, size)
                res = res + b'\x00'
                pos = pos + 1
            else:
                val = refs[a].to_bytes(ptr_size, 'little')  # get pointer
                res = res + val
                pos = pos + len(val)
        elif type(a) is int:
            val = a.to_bytes(ptr_size, 'little')
            res = res + val
            pos = pos + len(val)
        elif type(a) is tuple:
            val = a[0].to_bytes(a[1], 'little')
            res = res + val
            pos = pos + len(val)
        elif type(a) is bytes:
            res = res + a
            pos = pos + len(a)

    # fix relative offsets for jumps
    tmp = bytearray(res)
    for g in goto.keys():
        target_pos = labels[g]
        current_pos, size = goto[g]
        tmp[current_pos] = ctypes.c_ubyte(target_pos - current_pos - size).value
    res = bytes(tmp)
    return executable, res


def nasm(code, bits, origin, delete=True):
    with tempfile.NamedTemporaryFile(delete=False) as tmp_asm:
        tmp_asm.write(f'bits {bits}\norg {origin}\n{code}'.encode('utf-8'))
        tmp_asm.flush()

        tmp_bin_fd, tmp_bin_name = tempfile.mkstemp()
        os.close(tmp_bin_fd)

    try:
        p = subprocess.Popen(
            [
                'nasm',
                '-o', tmp_bin_name,
                '-f', 'bin',
                tmp_asm.name,
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        stdout, stderr = p.communicate()

        if p.returncode:
            raise SyntaxError(stderr.decode('utf-8'))

        tmp_bin = open(tmp_bin_name, 'rb')
        result = tmp_bin.read()
        tmp_bin.close()
        return result
    except FileNotFoundError as fn:
        raise Exception('nasm.exe must be in PATH environment variable!')
    finally:
        try:
            if delete:
                os.unlink(tmp_asm.name)
                os.unlink(tmp_bin_name)
        except OSError:
            pass
