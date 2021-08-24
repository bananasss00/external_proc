import sys
sys.path.append("..")  # use universal module from parent dir

from external_proc import *
from external_proc import core

PROCESS_NAME= 'Tutorial-x86_64.exe'
# Step 5: Code finder (PW=888899)

tutorial = 9

if tutorial == 6:
    # Step 6: Pointers: (PW=098712)
    with ExtProcess.ctx_open(PROCESS_NAME) as t:
        ptr = t.make_ptr(0x100306AD0, core.PtrType.Uint64).go_ptr()
        value = ptr.get_value()
        ptr.set_value(5000)
        print(value)

elif tutorial == 7:
    # Step 7: Code Injection: (PW=013370)
    with ExtProcess.ctx_open(PROCESS_NAME) as t:
        m = t.get_module()
        print(t.is_valid(), (m.base, m.size))
        code_decrement_health = 0x10002D4F7  #t.get_module().section('.text').find_pattern("8B B3 ? ? ? ? 83 AB", 6).get_address()
        print(f'code_decrement_health: {hex(code_decrement_health)}')  # 0x10002D4F7


        new_code = t.alloc(2048, code_decrement_health)  # alloc memory near 'code_decrement_health'
                                                         # for short relative jump!!!

        print(f'new_code: {hex(new_code)}; error: {get_last_error()}')

        t.virtual_protect(code_decrement_health, 7, PageFlags.PAGE_EXECUTE_READWRITE)
        t.write.bytes(code_decrement_health, nasm(f'''jmp {hex(new_code)}\nnop\nnop''', 64, hex(code_decrement_health)))
        t.write.bytes(new_code, nasm('''add dword [rsi+0x7E0], 0x2 ; +2 health instead -1
                                        jmp qword 0x10002D4FE''', 64, new_code))

elif tutorial == 8:
    # Step 8: Multilevel pointers: (PW=525927)
    with ExtProcess.ctx_open(PROCESS_NAME) as t:
        ptr = t.make_ptr(0x100306B00, core.PtrType.Uint32)\
            .go_ptr(0x10)\
            .go_ptr(0x18)\
            .go_ptr()\
            .go_ptr(0x18)

        print(ptr.get_value())
        ptr.set_value(5000)

elif tutorial == 9:
    # Step 9: Shared code: (PW=31337157)
    with ExtProcess.ctx_open(PROCESS_NAME) as t:
        # .text:10002EB68 EB 03   jmp short loc_10002EB6D
        code_health_mod = t.get_module().section('.text')\
            .find_pattern("EB 03 0F 28 C1")\
            .go_jmp_short_ptr()\
            .get_address()

        print(f'code_health_mod: {hex(code_health_mod)}')  # 0x10002EB6D

        new_code = t.alloc(address=code_health_mod)
        print(f'new_code: {hex(new_code)}')

        t.virtual_protect(code_health_mod, 5, PageFlags.PAGE_EXECUTE_READWRITE)
        shell_jmp_to_allocmem = nasm(f'jmp {hex(new_code)}', 64, code_health_mod)
        t.write.bytes(code_health_mod, shell_jmp_to_allocmem)

        # block decreasing health for player team
        t.write.bytes(new_code, nasm('''
                                        cmp dword [rbx+0x14], 1
                                        je @exit
                                        movss dword [rbx+0x8],xmm0 ; change health instruction
                                        @exit:
                                            jmp 0x10002EB72''',
                                     64, new_code))
