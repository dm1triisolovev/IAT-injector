#pragma once
#include "../operation/operation.hpp"

class c_communication {
public:
	static operation_callback init( std::string, std::string );
	static bool test_operation( operation_callback );
	static NTSTATUS copy_memory( operation_callback, ULONGLONG, uintptr_t, ULONGLONG, uintptr_t, SIZE_T );
	static uint64_t get_module_base( operation_callback, ULONGLONG, std::wstring );
};