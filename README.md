# external_proc python module
![Python Version](https://img.shields.io/pypi/pyversions/external-proc)
[![PYPI](https://img.shields.io/pypi/v/external-proc)](https://pypi.org/project/external-proc/)
[![Downloads](https://pepy.tech/badge/external-proc)](https://pepy.tech/project/external-proc)

## External process memory manager

# Installation
Ensure you have at least Python 3.6+
 ```
 pip install external_proc
 or
 pip install git+https://github.com/bananasss00/external_proc.git
 ```
 
# Usage examples
**More examples in 'tests' directory**

Open/Close process
```python
from external_proc import *

p = ExtProcess.open(PROCESS_NAME or PROCESS_ID)
p.close()
### or ###
with ExtProcess.ctx_open(PROCESS_NAME or PROCESS_ID) as p:
  pass
```

Read/Write values
```python
with ExtProcess.ctx_open(process_name) as p:
  # write
  p.write.list_bytes(address, [0x90, 0x90])
  p.write.bytes(address, b'\x90\x90')
  p.write.str(address, 'string')
  p.write.wstr(address, 'unicode string')
  p.write.uint8(address, 1)
  p.write.uint16(address, 1)
  p.write.uint32(address, 1)
  p.write.uint64(address, 1)
  p.write.int8(address, -1)
  p.write.int16(address, -1)
  p.write.int32(address, -1)
  p.write.int64(address, -1)
  p.write.float(address, 0.01)
  p.write.double(address, 0.01)
  # read
  v = p.read.list_bytes(address, BYTES_COUNT)
  v = p.read.bytes(address, BYTES_COUNT)
  v = p.read.str(address, MAX_BYTES_COUNT) # read string to first \x00
  v = p.read.wstr(address, MAX_BYTES_COUNT) # read string to first \x00
  v = p.read.uint8(address) # signed 1 byte value
  v = p.read.uint16(address) # signed 2 byte value
  v = p.read.uint32(address) # signed 4 byte value
  v = p.read.uint64(address) # signed 8 byte value
  v = p.read.int8(address) # unsigned 1 byte value
  v = p.read.int16(address) # unsigned 2 byte value
  v = p.read.int32(address) # unsigned 4 byte value
  v = p.read.int64(address) # unsigned 8 byte value
  v = p.read.float(address) # 4 byte
  v = p.read.double(address) # 8 byte
```

Pointers
```python
# Pointer types:
#   ListBytes
#   Bytes
#   Str
#   Wstr
#   Uint8
#   Uint16
#   Uint32
#   Uint64
#   Int8
#   Int16
#   Int32
#   Int64
#   Float
#   Double
#   Invalid
ptr = p.make_ptr(address, PtrType.Int32)
address = ptr.get_address() # return current address
ptr.set_value(333)
value = ptr.get_value()

# get address from multilevel pointers
ptr = p.make_ptr(0x6426E0, core.PtrType.Uint32)\
            .go_ptr(0xC)\
            .go_ptr(0x14)\
            .go_ptr()\
            .go_ptr(0x18)

# PtrTypes: ListBytes, Bytes, Str, Wstr
#   require additional argument
#   for get_value(BYTES_COUNT or MAX_BYTES_COUNT for strings)
```


Simple dll injector x32/x64
```python
with ExtProcess.ctx_open(process_name) as p:
  dll_path = os.path.abspath('lib.dll')
  loadlib_func = get_proc_address('kernel32', 'LoadLibraryA', x64=p.is_x64_process())
  param = p.alloc()
  p.write.str(param, dll_path)
  with p.ctx_create_thread(loadlib, param, wait_thread=True) as th_id:
      pass
```

Signature scanner. IDA Style
```python
exe_module = p.get_module()
client_module = p.get_module('client.dll')

# .text:00428873 8D 4D F0          lea ecx, [ebp+var_10]
# .text:00428876 E8 05 4E FE FF    call 0x40D680

# E8 ? ? ? ? - it's instruction call 0x40D680
signature = "8D 4D F0 E8 ? ? ? ?" 

# equal: find_pattern(signature) + 3
sig_in_all_module: Ptr = client_module.find_pattern(signature, add_offset=3)
sig_in_code_section: Ptr = client_module.section('.text').find_pattern(signature, 3)

# for read relative offset from call instruction you can simple do this
adr = sig_in_code_section.go_call_ptr().get_address()
# same for jmp, je and etc inctructions: .go_jmp_ptr(), .go_jmp_short_ptr()
```
 

