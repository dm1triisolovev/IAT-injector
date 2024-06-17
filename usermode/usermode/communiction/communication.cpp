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