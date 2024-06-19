#include "memory.hpp"

#include "../communiction/communication.hpp"

NTSTATUS memory_t::read_buffer( uint64_t address, LPVOID lp_buffer, SIZE_T size ) {
	if( lp_buffer == 0 )
		return STATUS_INVALID_PARAMETER;

	return c_communication::copy_memory( this->operation_callback, process_id, address, GetCurrentProcessId( ), uintptr_t( lp_buffer ), size );
}


