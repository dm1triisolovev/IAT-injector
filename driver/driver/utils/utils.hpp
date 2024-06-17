#pragma once
#include "kernel/kernel.hpp"

namespace utils {
	uintptr_t find_pattern( const uintptr_t, const size_t, const char*, const char* );
	uintptr_t find_pattern( uintptr_t, LPCSTR, LPCSTR );
	uintptr_t get_system_base( LPCSTR, PULONG );
}