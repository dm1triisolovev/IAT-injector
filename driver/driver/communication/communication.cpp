#include "communication.hpp"
#include "../utils/utils.hpp"

#define RVA(addr, size)			((PBYTE)(addr + *(DWORD*)(addr + ((size) - 4)) + size))

fn_operation_callback o_operation_callback = nullptr;

PVOID c_communication::operation_callback( packet_base_t& packet, PBYTE a2 )
{
	DbgPrint( "[ Driver ] Test is working!!!\n" );

	return o_operation_callback( packet, a2 );
}

NTSTATUS c_communication::setup( ) {

	ULONG ntoskrnl_size = 0;
	auto ntoskrnl_base = utils::get_system_base( 0, &ntoskrnl_size );

	auto target = utils::find_pattern(
		( PVOID )ntoskrnl_base,
		"\x4C\x8B\x05\x00\x00\x00\x00\x33\xC0\x4D\x85\xC0\x74\x08\x49\x8B\xC0\xE8\x00\x00\x00\x00\xF7\xD8",
		"xxx????xxxxxxxxxxx????xx"
	);

	if( !target ) {
		DbgPrint( "[ Driver ] Unable to find signature!\n" );
		return false;
	}

	target = RVA( target, 7 );

	*(PVOID* )&o_operation_callback = InterlockedExchangePointer(( volatile PVOID* )target, operation_callback);

	return true;
}