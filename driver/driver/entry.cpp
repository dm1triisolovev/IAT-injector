#include "communication/communication.hpp"

#pragma comment(linker, "/ENTRY:JordEntry")
#pragma comment(lib, "ntoskrnl.lib")

NTSTATUS JordEntry( ) {
	DbgPrint( "[ Driver ] Driver loaded!\n" );

	if( !c_communication::setup( ) ) {
		DbgPrint( "[ Driver ] Failed to ebanle communication.\n" );
		return STATUS_UNSUCCESSFUL;
	}

	DbgPrint( "[ NESTELL REBORN ] Communication successfully enabled!\n" );

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry( PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath ) {
	return STATUS_SUCCESS;
}