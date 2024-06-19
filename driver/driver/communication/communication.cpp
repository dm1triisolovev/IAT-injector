#include "communication.hpp"

#include "../utils/utils.hpp"

#include "../memory/memory.hpp"

#define RVA(addr, size)			((PBYTE)(addr + *(DWORD*)(addr + ((size) - 4)) + size))

fn_operation_callback o_operation_callback = nullptr;

BOOL c_communication::operation_callback( packet_base_t& packet, ULONG64 magic )
{
	if( magic == 0xDEADBEEF ) {
		if( packet.opcode == NONE ) {
			DbgPrint( "[ Driver ] packet is none\n" );
			return false;
		}

		if( packet.side != e_side::SERVER ) {
			DbgPrint( "[ Driver ] packet side is not server\n" );
			return false;
		}

		switch( packet.opcode ) {
			case e_opcode::TEST: {
				RtlSecureZeroMemory( &packet.client, sizeof( packet.client ) );

				packet.side = e_side::CLIENT;
				packet.client.test.is_valid = true;

				return true;
			}
			case e_opcode::COPY_VIRTUAL_MEMORY: {
				auto [target_pid, target_address, source_pid, source_address, size] = packet.server.copy_memory;
				auto status = c_memory::copy_memory( source_pid, target_pid, source_address, target_address, size );

				RtlSecureZeroMemory( &packet.client, sizeof( packet.client ) );

				packet.side = e_side::CLIENT;
				packet.client.copy_memory.size = status;

				return NT_SUCCESS( status );
			}
			case GET_MODULE_BASE_SIZE: {
				auto [pid, name] = packet.server.get_module;
				auto base = utils::get_module_base( pid, name );

				RtlSecureZeroMemory( &packet.client, sizeof( packet.client ) );

				packet.side = e_side::CLIENT;

				packet.client.get_module.base_address = base;
				packet.client.get_module.module_size = 0x1702;

				return true;
			}
			case ALLOC_VIRTUAL_MEMORY: {
				auto [target_pid, allocation_type, protect, source_address, target_address, size, code] = packet.server.alloc_memory;
				auto status = c_memory::allocate_virtual_memory( target_pid, ( PVOID* )&source_address, size, allocation_type, protect );

				RtlSecureZeroMemory( &packet.client, sizeof( packet.client ) );

				packet.side = e_side::CLIENT;

				packet.client.alloc_memory.target_address = source_address;
				packet.client.alloc_memory.code = status;

				return NT_SUCCESS( status );
			}
			case PROTECT_VIRTUAL_MEMORY: {
				auto [target_pid, protect, source_address, size, code] = packet.server.protect_memory;
				auto status = c_memory::protect_virtual_memory( target_pid, ( PVOID )source_address, size, protect );

				RtlSecureZeroMemory( &packet.client, sizeof( packet.client ) );

				packet.side = e_side::CLIENT;

				packet.client.protect_memory.code = status;
				packet.client.protect_memory.protect = protect;

				return NT_SUCCESS( status );
			}
			case FREE_VIRTUAL_MEMORY: {
				auto [target_pid, address, code] = packet.server.free_memory;
				auto status = c_memory::free_virtual_memory( target_pid, ( PVOID )address );

				RtlSecureZeroMemory( &packet.client, sizeof( packet.client ) );

				packet.side = e_side::CLIENT;

				packet.client.free_memory.code = status;

				return NT_SUCCESS( status );
			}
			default:
				return false;
		}
	}

	return o_operation_callback( packet, magic );
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