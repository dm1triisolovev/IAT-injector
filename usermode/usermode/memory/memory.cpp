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