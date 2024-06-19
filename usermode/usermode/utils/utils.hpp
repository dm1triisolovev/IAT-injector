#pragma once
#include "../stdafx/stdafx.hpp"

namespace utils {
	DWORD get_proc_id( std::string );
	PIMAGE_SECTION_HEADER get_enclosing_section_header( uint64_t, PIMAGE_NT_HEADERS );
	uint64_t* get_ptr_from_rva( uint64_t, IMAGE_NT_HEADERS*, uint8_t* );
	void solve_relocations( uint64_t, uint64_t, IMAGE_NT_HEADERS*, IMAGE_BASE_RELOCATION*, size_t );
	bool parse_imports( uint64_t );
	void solve_imports( uint8_t*, IMAGE_NT_HEADERS*, IMAGE_IMPORT_DESCRIPTOR* );
}