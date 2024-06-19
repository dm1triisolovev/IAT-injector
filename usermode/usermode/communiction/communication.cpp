#include "communication.hpp"

operation_callback c_communication::init( std::string module_name, std::string routin_name ) {
	auto h_module = LoadLibraryA( module_name.c_str( ) );
	if( !h_module ) {
		printf( "[ Usermode ] Communication init error: Failed to load library.\n" );
		return nullptr;
	}

	operation_callback callback = ( operation_callback )GetProcAddress( h_module, routin_name.c_str( ) );

	return callback;
}

bool c_communication::test_operation( operation_callback operation ) {
	packet_base_t packet{};

	packet.opcode = e_opcode::TEST;
	packet.side = e_side::SERVER;

	operation( packet, 0xDEADBEEF );

	if( packet.client.test.is_valid )
		return true;

	return false;
}

NTSTATUS c_communication::copy_memory( operation_callback operation, ULONGLONG src_pid, uintptr_t src_addr, ULONGLONG target_pid, uintptr_t target_addr, SIZE_T size ) {
	packet_base_t packet{};

	packet.opcode = e_opcode::COPY_VIRTUAL_MEMORY;
	packet.side = e_side::SERVER;

	auto& server_req = packet.server.copy_memory;

	server_req.source_pid = src_pid;
	server_req.source_address = src_addr;

	server_req.target_pid = target_pid;
	server_req.target_address = target_addr;

	server_req.size = size;

	operation( packet, 0xDEADBEEF );

	auto client_req = packet.client.copy_memory;

	auto status = NTSTATUS( client_req.size );

	return status;
}