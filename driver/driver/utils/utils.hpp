#pragma once
#include "kernel/kernel.hpp"

namespace utils {
	PBYTE find_pattern( PVOID module, DWORD size, LPCSTR pattern, LPCSTR mask );
	PBYTE find_pattern( PVOID base, LPCSTR pattern, LPCSTR mask );
	uintptr_t get_system_base( LPCSTR, PULONG );
}