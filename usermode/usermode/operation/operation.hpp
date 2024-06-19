#pragma once
#include "../stdafx/stdafx.hpp"

enum e_opcode : BYTE {
	NONE, 
	TEST,
	COPY_VIRTUAL_MEMORY,
	GET_MODULE_BASE_SIZE,
	ALLOC_VIRTUAL_MEMORY,
	PROTECT_VIRTUAL_MEMORY,
	FREE_VIRTUAL_MEMORY,
};

enum class e_side : BYTE {
	NONE,
	CLIENT,
	SERVER,
};

struct server_test{};
struct client_test {
	bool is_valid = false;
};

struct copy_virtual_memory_server_t {
	ULONGLONG target_pid;
	uintptr_t target_address;
	ULONGLONG source_pid;
	uintptr_t source_address;
	size_t size;
};

struct copy_virtual_memory_client_t {
	ULONGLONG target_pid;
	uintptr_t target_address;
	ULONGLONG source_pid;
	uintptr_t source_address;
	size_t size;
};

struct get_module_server_t {
	ULONGLONG pid;
	wchar_t name[32];
};

struct get_module_client_t {
	uintptr_t base_address;
	size_t module_size;
};

struct alloc_virtual_memory_server_t {
	ULONG target_pid, allocation_type, protect;
	uintptr_t source_address;
	uintptr_t target_address;
	size_t size;
	size_t code;
};

struct alloc_virtual_memory_client_t {
	ULONG target_pid, allocation_type, protect;
	uintptr_t source_address;
	uintptr_t target_address;
	size_t size;
	size_t code;
};

struct protect_virtual_memory_server_t {
	ULONG target_pid, protect;
	uintptr_t source_address;
	size_t size;
	size_t code;
};

struct protect_virtual_memory_client_t {
	ULONG target_pid, protect;
	uintptr_t source_address;
	size_t size;
	size_t code;
};

struct free_virtual_memory_server_t {
	ULONG target_pid;
	uintptr_t address;
	size_t code;
};

struct free_virtual_memory_client_t {
	ULONG target_pid;
	uintptr_t address;
	size_t code;
};

struct packet_base_t {
	e_opcode opcode;
	e_side side;

	union {
		union {
			server_test test;

			copy_virtual_memory_server_t copy_memory;
			get_module_server_t get_module;
			alloc_virtual_memory_server_t alloc_memory;
			protect_virtual_memory_server_t protect_memory;
			free_virtual_memory_server_t free_memory;
		} server;

		union {
			client_test test;

			copy_virtual_memory_client_t copy_memory;
			get_module_client_t get_module;
			alloc_virtual_memory_client_t alloc_memory;
			protect_virtual_memory_client_t protect_memory;
			free_virtual_memory_client_t free_memory;
		} client;
	};
};

typedef BOOL( *operation_callback )( packet_base_t&, ULONG64 );