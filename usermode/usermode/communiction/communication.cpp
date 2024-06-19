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

bool c_communication::test_operation( operation_callback opertaion ) {
	packet_base_t packet{};

	packet.opcode = TEST;
	packet.side = e_side::SERVER;

	opertaion( packet, 0xDEADBEEF );

	return packet.client.test.is_valid;
}