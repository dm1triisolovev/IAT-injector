#pragma once

#include "../operation/operation.hpp"

#include <memory>
#include <vector>
#include <string>
#include <winternl.h>

struct memory_t {
	NTSTATUS read_buffer( uint64_t, LPVOID, SIZE_T );
	NTSTATUS write_memory( uint64_t address, uintptr_t dstAddress, SIZE_T nSize );

	uint64_t process_id = 0;
	operation_callback operation_callback;
};

extern std::unique_ptr<memory_t> memory;