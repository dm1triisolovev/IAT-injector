#pragma once

#include "../operation/operation.hpp"

#include <memory>
#include <vector>
#include <string>
#include <winternl.h>

struct memory_t {
	NTSTATUS read_buffer( uint64_t, LPVOID, SIZE_T );

	uint64_t process_id = 0;
	operation_callback operation_callback;
};

extern std::unique_ptr<memory_t> memory;