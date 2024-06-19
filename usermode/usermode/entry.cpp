#include "communiction/communication.hpp"

#include "utils/utils.hpp"

int main( ) {

	operation_callback operation = c_communication::init( "ntdll.dll", "NtCompareSigningLevels" );
	if( !operation ) {
		printf( "[ Usermode ] Failed to init operation callback" );
		Sleep( 5000 );
		exit( -1 );
	}

	printf( "[ Usermode ] Success!\n\n" );

	bool status = c_communication::test_operation( operation );
	if( !status ) {
		printf( "[ Usermode ] map driver.sys first\n" );
	}

	DWORD pid = utils::get_proc_id( "" );
	if( !pid ) {
		printf( "\nWaiting for process..." );

		for( ;; Sleep( 200 ) ) {
			pid = utils::get_proc_id( "" );
			if( pid )
				break;
		}
	}



	std::cin.get( );
	return 0;
}