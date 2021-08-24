from typing import Tuple, Union, List, Iterator, ContextManager, Any, NoReturn, Literal
import external_proc._external_proc as core
from enum import Enum


def get_last_error() -> int:
	pass

class PtrType(Enum):
	ListBytes = 0
	Bytes = 1
	Str = 2
	Wstr = 3
	Uint8 = 4
	Uint16 = 5
	Uint32 = 6
	Uint64 = 7
	Int8 = 8
	Int16 = 9
	Int32 = 10
	Int64 = 11
	Float = 12
	Double = 13
	Invalid = 14

class Ptr:
	def __init__(self, process: 'Process', ptr: int, type: PtrType):
		pass

	def is_valid(self) -> bool:
		pass

	def is_memory_readable(self) -> bool:
		pass

	def set_ptr_type(self, type: PtrType) -> Ptr:
		pass

	def get_value(self, value_size: int = 0) -> Union[str, int, bytes]:
		pass

	def set_value(self, value) -> bool:
		pass

	def get_address(self) -> int:
		'''

		:return: address
		'''
		pass

	def go_ptr(self, add_offset: int = 0) -> Ptr:
		pass

	def go_relative_ptr(self, instruction_offset: int, instruction_size: int, relative_adr_size: Literal[1, 2, 4, 8], add_offset: int = 0) -> Ptr:
		pass

	def go_call_ptr(self, add_offset: int = 0) -> Ptr:
		pass

	def go_jmp_ptr(self, add_offset: int = 0) -> Ptr:
		pass

	def go_jmp_short_ptr(self, add_offset: int = 0) -> Ptr:
		pass


class PeSection:
	# !not have ctor!
	def __init__(self):
		self.base: int = None
		self.size: int = None

	def is_valid(self) -> bool:
		pass

	def find_pattern(self, signature: str, add_offset: int = 0) -> Ptr:
		pass


class ProcessModule:
	# !not have ctor!
	def __init__(self):
		self.base: int = None
		self.size: int = None

	def is_valid(self) -> bool:
		pass

	def find_pattern(self, signature: str, add_offset: int = 0) -> Ptr:
		pass

	def section(self, name: str) -> PeSection:
		pass


class Reader:
	def __init__(self, process_handle: int):
		pass
	def relative_offset_to_absolute(self, instruction_address: int, relative_adr_offset: int, instruction_size: int, relative_adr_size: int) -> int:
		pass
	def list_bytes(self, address: int, size: int) -> List[int]:
		pass
	def bytes(self, address: int, size: int) -> bytes:
		pass
	def str(self, address: int, max_size: int) -> str:
		pass
	def wstr(self, address: int, max_size: int) -> str:
		pass
	def uint8(self, address: int) -> int:
		pass
	def uint16(self, address: int) -> int:
		pass
	def uint32(self, address: int) -> int:
		pass
	def uint64(self, address: int) -> int:
		pass
	def int8(self, address: int) -> int:
		pass
	def int16(self, address: int) -> int:
		pass
	def int32(self, address: int) -> int:
		pass
	def int64(self, address: int) -> int:
		pass
	def float(self, address: int) -> float:
		pass
	def double(self, address: int) -> double:
		pass


class Writer:
	def __init__(self, process_handle: int):
		pass
	def list_bytes(self, address: int, bytes: list[int]) -> bool:
		pass
	def bytes(self,  address: int, bytes: bytes) -> bool:
		pass
	def str(self, address: int, string: str) -> bool:
		pass
	def wstr(self, address: int, string: str) -> bool:
		pass
	def uint8(self, address: int, value: int) -> bool:
		pass
	def uint16(self, address: int, value: int) -> bool:
		pass
	def uint32(self, address: int, value: int) -> bool:
		pass
	def uint64(self, address: int, value: int) -> bool:
		pass
	def int8(self, address: int, value: int) -> bool:
		pass
	def int16(self, address: int, value: int) -> bool:
		pass
	def int32(self, address: int, value: int) -> bool:
		pass
	def int64(self, address: int, value: int) -> bool:
		pass
	def float(self, address: int, value: float) -> bool:
		pass
	def double(self, address: int, value: double) -> bool:
		pass

class Process:
	def __init__(self, process_id_or_name: Union[int, str]):
		self.read: Reader = None
		self.write: Writer = None
		self._process_handle = None
		pass
	def is_valid(self) -> bool:
		pass
	def close(self) -> NoReturn:
		pass
	# @staticmethod
	# def open_by_id(id: int) -> Process:
	# 	pass
	# @staticmethod
	# def open_by_name(process_name: str) -> list[Process]:
	# 	pass
	@staticmethod
	def get_process_id(process_name: str) -> int:
		pass
	@staticmethod
	def get_process_ids(pprocess_name: str) -> List[int]:
		pass
	def open_process(self, process_id: int) -> NoReturn:
		pass
	def close_handle(self, handle: int) -> NoReturn:
		pass
	def get_module(self, name: str = '') -> ProcessModule:
		'''

		:param name: if empty used main module(process name)
		:return: ProcessModule
		'''
		pass
	def get_exe_name(self) -> str:
		pass
	def alloc(self, size: int = 4096, address: int = 0, fl_protect: int = core.PAGE_EXECUTE_READWRITE) -> int:
		pass
	def free(self, address: int) -> bool:
		pass
	def virtual_protect(self, address: int, size: int, flags: int) -> int:
		pass
	def create_thread(self, address: int, parameter: int = 0) -> int:
		pass
	def wait_for_single_object(self, handle: int):
		pass
	def get_pe_section(self, module_ptr: int, section_name: str) -> PeSection:
		pass
	def is_x64_process(self) -> bool:
		pass
	def find_pattern(self, scan_start: int, scan_end: int, signature: str, add_offset: int = 0) -> Ptr:
		pass
	def make_ptr(self, address: int, type: PtrType, value_size: int = 0) -> Ptr:
		pass
	def is_memory_readable(self, address: int) -> bool:
		pass


