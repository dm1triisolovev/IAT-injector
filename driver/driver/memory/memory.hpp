#pragma once
#include "../utils/kernel/kernel.hpp"

class c_memory {
public:
	static NTSTATUS copy_memory( uint32_t current_proc_id, uint32_t target_proc_id, uint64_t current_dest_addr, uint64_t target_dest_address, uint32_t mem_size );
	static NTSTATUS allocate_virtual_memory( uint32_t target_proc_id, PVOID* base_address, SIZE_T size, ULONG allocation_type, ULONG protect );
	static NTSTATUS protect_virtual_memory( uint32_t target_proc_id, PVOID base_address, SIZE_T size, ULONG new_protect );
	static NTSTATUS free_virtual_memory( uint32_t target_proc_id, PVOID base_address );
};