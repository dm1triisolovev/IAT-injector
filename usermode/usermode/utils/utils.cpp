#include "utils.hpp"

#include <TlHelp32.h>

DWORD utils::get_proc_id( std::string proc_name ) {
	if( proc_name.empty( ) )
		return NULL;

	HANDLE h_snap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );

	if( h_snap == INVALID_HANDLE_VALUE )
		return NULL;

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof( pe );

	if( Process32First( h_snap, &pe ) ) {
		while( Process32Next( h_snap, &pe ) ) {
			if( !strcmp( pe.szExeFile, proc_name.c_str( ) ) ) {
				CloseHandle( h_snap );
				return pe.th32ProcessID;
			}
		}
	}

	CloseHandle( h_snap );

	return NULL;
}