#include "memory.hpp"

#include "../communiction/communication.hpp"

NTSTATUS memory_t::read_buffer( uint64_t address, LPVOID lp_buffer, SIZE_T size ) {
	if( lp_buffer == 0 )
		return STATUS_INVALID_PARAMETER;

	return c_communication::copy_memory( operation_callback, process_id, address, GetCurrentProcessId( ), uintptr_t( lp_buffer ), size );
}

NTSTATUS memory_t::write_memory( uint64_t address, uintptr_t dst_address, SIZE_T size ) {
	if( dst_address == 0 )
		return STATUS_INVALID_PARAMETER;

	return c_communication::copy_memory( operation_callback, GetCurrentProcessId( ), dst_address, process_id, address, size );
}

uint64_t memory_t::read_chain( uint64_t base, const std::vector<uint64_t>& offsets ) {
	uint64_t result = read<uint64_t>( base + offsets.at( 0 ) );

	for( int i = 1; i < offsets.size( ); i++ )
		result = read<uint64_t>( result + offsets.at( i ) );

	return result;
}

uint64_t memory_t::get_module( std::wstring module_name ) {
	return c_communication::get_module_base( operation_callback, process_id, module_name );
}

uint64_t memory_t::allocate( size_t size, uint32_t allocation_type, uint32_t protect ) {
	uint64_t address = 0;
	return c_communication::alloc_memory( operation_callback, process_id, size, allocation_type, protect, address );
}

NTSTATUS memory_t::protect( uint64_t address, size_t size, uint32_t protect ) {
	return c_communication::protect_memory( operation_callback, process_id, size, protect, address );
}

NTSTATUS memory_t::free( uint64_t address ) {
	return c_communication::free_memory( operation_callback, process_id, address );
}

std::unique_ptr<memory_t> memory = std::make_unique<memory_t>( );