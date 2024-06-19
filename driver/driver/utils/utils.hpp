#pragma once
#include "kernel/kernel.hpp"

namespace utils {
	PBYTE find_pattern( PVOID, DWORD, LPCSTR, LPCSTR );
	PBYTE find_pattern( PVOID, LPCSTR, LPCSTR  );
	uintptr_t get_system_base( LPCSTR, PULONG );
	uint64_t get_module_base( ULONGLONG, LPCWSTR );
}