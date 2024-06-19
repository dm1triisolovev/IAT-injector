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

uint64_t c_communication::get_module_base( operation_callback operation, ULONGLONG process_id, std::wstring module_name ) {
	packet_base_t packet{};

	packet.opcode = e_opcode::GET_MODULE_BASE_SIZE;
	packet.side = e_side::SERVER;

	auto& server_req = packet.server;
	module_name.copy( server_req.get_module.name, module_name.length( ) );

	server_req.get_module.pid = process_id;

	operation( packet, 0xDEADBEEF );

	auto client_req = packet.client.get_module;

	auto base_address = client_req.base_address;

	return base_address;
}

uint64_t c_communication::alloc_memory( operation_callback operation, ULONGLONG target_pid, size_t size, uint32_t allocation_type, uint32_t protect, uintptr_t source_address ) {
	packet_base_t packet{};

	packet.opcode = e_opcode::ALLOC_VIRTUAL_MEMORY;
	packet.side = e_side::SERVER;

	auto& server_req = packet.server.alloc_memory;

	server_req.target_pid = target_pid;
	server_req.source_address = source_address;

	server_req.allocation_type = allocation_type;
	server_req.protect = protect;

	server_req.size = size;
	server_req.code = STATUS_INTERRUPTED;

	operation( packet, 0xDEADBEEF );

	auto client_req = packet.client.alloc_memory;

	auto address = client_req.target_address;

	return address;
}

NTSTATUS c_communication::protect_memory( operation_callback operation, ULONGLONG target_pid, size_t size, uint32_t protect, uintptr_t source_address ) {
	packet_base_t packet{};

	packet.opcode = e_opcode::PROTECT_VIRTUAL_MEMORY;
	packet.side = e_side::SERVER;

	auto& server_req = packet.server.protect_memory;

	server_req.target_pid = target_pid;
	server_req.source_address = source_address;

	server_req.protect = protect;

	server_req.size = size;
	server_req.code = STATUS_INTERRUPTED;

	operation( packet, 0xDEADBEEF );

	auto client_req = packet.client.protect_memory;

	protect = client_req.protect;

	auto status = NTSTATUS( client_req.code );

	return status;
}
