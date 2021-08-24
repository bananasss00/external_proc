import sys
sys.path.append("..")  # use universal module from parent dir
from external_proc import *
from external_proc import core

PROCESS_NAME= 'Tutorial-i386.exe'
# Step 5: Code finder (PW=888899)

tutorial = 9

if tutorial == 6:
    # Step 6: Pointers: (PW=098712)
    with ExtProcess.ctx_open(PROCESS_NAME) as t:
        ptr = t.make_ptr(0x006426B0, core.PtrType.Uint32).go_ptr()
        value = ptr.get_value()
        ptr.set_value(5000)
        print(value)

elif tutorial == 7:
    # Step 7: Code Injection: (PW=013370)
    with ExtProcess.ctx_open(PROCESS_NAME) as t:
        new_code = t.alloc()
        print(f'new_code: {hex(new_code)}')

        code_decrement_health = t.get_module().section('.text').find_pattern("8B B3 ? ? ? ? 83 AB", 6).get_address()
        print(f'code_decrement_health: {hex(code_decrement_health)}')  # 0x4275E3

        t.virtual_protect(code_decrement_health, 7, PageFlags.PAGE_EXECUTE_READWRITE)
        t.write.bytes(code_decrement_health, nasm(f'jmp dword {hex(new_code)}\nnop\nnop', 32, code_decrement_health))
        t.write.bytes(new_code, nasm('''add dword [ebx+0x4A4], 0x2 ; +2 health instead -1
                                        jmp dword 0x004275EA''', 32, new_code))

elif tutorial == 8:
    # Step 8: Multilevel pointers: (PW=525927)
    with ExtProcess.ctx_open(PROCESS_NAME) as t:
        ptr = t.make_ptr(0x6426E0, core.PtrType.Uint32)\
            .go_ptr(0xC)\
            .go_ptr(0x14)\
            .go_ptr()\
            .go_ptr(0x18)

        print(ptr.get_value())
        ptr.set_value(5000)

elif tutorial == 9:
    # Step 9: Shared code: (PW=31337157)
    with ExtProcess.ctx_open(PROCESS_NAME) as t:
        new_code = t.alloc()
        print(f'new_code: {hex(new_code)}')

        code_health_mod = t.get_module().section('.text').find_pattern("89 43 04 D9 EE").get_address()
        print(f'code_health_mod: {hex(code_health_mod)}')  # 0x4288d9

        t.virtual_protect(code_health_mod, 5, PageFlags.PAGE_EXECUTE_READWRITE)
        shell_jmp_to_allocmem = nasm(f'jmp dword {hex(new_code)}', 32, code_health_mod)
        t.write.bytes(code_health_mod, shell_jmp_to_allocmem)

        # block decreasing health for player team
        t.write.bytes(new_code, nasm('''
                                        cmp dword [ebx+0x10], 1
                                        je @exit
                                        mov dword [ebx+4], eax ; change health instruction
                                        @exit:
                                            fldz ; restore overrided bytes
                                            jmp dword 0x4288DE''',
                                     32, new_code))


# tutorial8()
