#include "communication.hpp"
#include "../utils/utils.hpp"

#define RVA(addr, size)			((uintptr_t)(addr + *(DWORD*)(addr + ((size) - 4)) + size))

PVOID c_communication::operation_callback( packet_base_t& packet, PBYTE a2 )
{
	DbgPrint( "[ Driver ] Test is working!!!\n" );

	return o_operation_callback( packet, a2 );
}

NTSTATUS c_communication::setup( LPCSTR module_name, LPCSTR ) {
	auto module_base = utils::get_system_base( module_name, 0 );

	if( !module_base ) {
		DbgPrint( "[ Driver ] Exported routine not found.\n" );
		return false;
	}

	auto target = utils::find_pattern(
		module_base,
		"\x4C\x8B\x05\x00\x00\x00\x00\x33\xC0\x4D\x85\xC0\x74\x08\x49\x8B\xC0\xE8\x00\x00\x00\x00\xF7\xD8",
		"xxx????xxxxxxxxxxx????xx"
	);

	if( !target ) {
		DbgPrint( "[ Driver ] Unable to find signature!\n" );
		return false;
	}

	target = RVA( target, 7 );

	*(PVOID* )&o_operation_callback = InterlockedExchangePointer(( volatile PVOID* )target, operation_callback);
}