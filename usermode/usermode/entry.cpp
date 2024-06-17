#include "communiction/communication.hpp"

int main( ) {

	operation_callback operation = c_communication::init( "ntdll.dll", "NtCompareSigningLevels" );
	if( !operation ) {
		printf( "[ Usermode ] Failed to init operation callback" );
		Sleep( 5000 );
		exit( -1 );
	}

	printf( "[ Usermode ] Success!\n\n" );


	std::cin.get( );
	return 0;
}