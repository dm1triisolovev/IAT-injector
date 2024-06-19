#pragma once

#include "../operation/operation.hpp"

#include <memory>
#include <vector>
#include <string>
#include <winternl.h>

struct memory_t {
	NTSTATUS read_buffer( uint64_t, LPVOID, SIZE_T );
	NTSTATUS write_memory( uint64_t, uintptr_t, SIZE_T );

	template<typename T>
	T read( uint64_t address );

	uint64_t process_id = 0;
	operation_callback operation_callback;
};

template<typename T>
inline T memory_t::read( uint64_t address ) {
	T buffer{};
	read_buffer( address, &buffer, sizeof( T ) );
	return buffer;
}

extern std::unique_ptr<memory_t> memory;