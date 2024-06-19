#include "utils.hpp"

#include <TlHelp32.h>

#include "../memory/memory.hpp"

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

PIMAGE_SECTION_HEADER utils::get_enclosing_section_header( uint64_t rva, PIMAGE_NT_HEADERS nt_header ) {
	PIMAGE_SECTION_HEADER section{ IMAGE_FIRST_SECTION( nt_header ) };

	for( int i = 0; i < nt_header->FileHeader.NumberOfSections; i++, section++ ) {
		uint64_t size{ section->Misc.VirtualSize };

		if( !size )
			size = section->SizeOfRawData;

		if( ( rva >= section->VirtualAddress ) && ( rva < ( section->VirtualAddress + size ) ) )
			return section;
	}

	return 0;
}

uint64_t* utils::get_ptr_from_rva( uint64_t rva, IMAGE_NT_HEADERS* nt_header, uint8_t* image_base ) {
	PIMAGE_SECTION_HEADER section_header{ get_enclosing_section_header( rva, nt_header ) };

	if( !section_header )
		return 0;

	int64_t delta{ ( int64_t )( section_header->VirtualAddress - section_header->PointerToRawData ) };

	return ( uint64_t* )( image_base + rva - delta );
}

void utils::solve_relocations( uint64_t base, uint64_t relocation_base, IMAGE_NT_HEADERS* nt_header, IMAGE_BASE_RELOCATION* reloc, size_t size ) {
	uint64_t image_base{ nt_header->OptionalHeader.ImageBase };
	uint64_t delta{ relocation_base - image_base };

	unsigned int bytes{ 0 };

	while( bytes < size ) {
		uint64_t* reloc_base{ ( uint64_t* )get_ptr_from_rva( ( uint64_t )( reloc->VirtualAddress ), nt_header, ( PBYTE )base ) };

		auto num_of_relocations{ ( reloc->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION ) ) / sizeof( WORD ) };
		auto reloc_data = ( uint16_t* )( ( uint64_t )reloc + sizeof( IMAGE_BASE_RELOCATION ) );

		for( unsigned int i = 0; i < num_of_relocations; i++ ) {
			if( ( ( *reloc_data >> 12 ) & IMAGE_REL_BASED_HIGHLOW ) )
				*( uint64_t* )( ( uint64_t )reloc_base + ( ( uint64_t )( *reloc_data & 0x0FFF ) ) ) += delta;

			reloc_data++;
		}

		bytes += reloc->SizeOfBlock;
		reloc = ( IMAGE_BASE_RELOCATION* )reloc_data;
	}

	return;
}

bool utils::parse_imports( uint64_t module_base ) {
	auto dos_header{ memory->read< IMAGE_DOS_HEADER >( module_base ) };
	auto nt_headers{ memory->read< IMAGE_NT_HEADERS >( module_base + dos_header.e_lfanew ) };
	auto descriptor{ memory->read< IMAGE_IMPORT_DESCRIPTOR >( module_base + nt_headers.OptionalHeader.DataDirectory[1].VirtualAddress ) };

	int descriptor_count{ 0 };
	int thunk_count{ 0 };

	while( descriptor.Name ) {
		auto first_thunk{ memory->read< IMAGE_THUNK_DATA >( module_base + descriptor.FirstThunk ) };
		auto original_first_thunk{ memory->read< IMAGE_THUNK_DATA >( module_base + descriptor.OriginalFirstThunk ) };
		thunk_count = 0;

		while( original_first_thunk.u1.AddressOfData ) {
			char name[256];
			memory->read_buffer( module_base + original_first_thunk.u1.AddressOfData + 0x2, ( LPVOID )name, 256 );

			std::string str_name( name );
			auto thunk_offset{ thunk_count * sizeof( uintptr_t ) };

			if( str_name.length( ) > 0 )
				memory->imports[str_name] = module_base + descriptor.FirstThunk + thunk_offset;

			++thunk_count;
			first_thunk = memory->read< IMAGE_THUNK_DATA >( module_base + descriptor.FirstThunk + sizeof( IMAGE_THUNK_DATA ) * thunk_count );
			original_first_thunk = memory->read< IMAGE_THUNK_DATA >( module_base + descriptor.OriginalFirstThunk + sizeof( IMAGE_THUNK_DATA ) * thunk_count );
		}

		++descriptor_count;
		descriptor = memory->read< IMAGE_IMPORT_DESCRIPTOR >( module_base + nt_headers.OptionalHeader.DataDirectory[1].VirtualAddress + sizeof( IMAGE_IMPORT_DESCRIPTOR ) * descriptor_count );
	}

	return ( memory->imports.size( ) > 0 );
}

std::wstring ansi_to_wstring( const std::string& input, DWORD locale = CP_UTF8 ) {
	wchar_t buf[8192] = { 0 };
	MultiByteToWideChar( locale, 0, input.c_str( ), ( int )input.length( ), buf, ARRAYSIZE( buf ) );
	return buf;
}

uint64_t GetFuncAddress( const char* module_name, const char* func ) {
	std::wstring module_name_utf = ansi_to_wstring( std::string( module_name ) );

	uint64_t remote_module{ memory->get_module( module_name_utf ) };

	if( !remote_module ) {
		printf("[ Usermode ] Failed to get module base from: %ws\n", module_name_utf.c_str( ) );
		exit( -1 );
	}

	uint64_t local_module{ ( uint64_t )GetModuleHandleA( module_name ) };

	if( !local_module ) {
		printf( "[Usermode] Failed to get module handle from: %s\n", module_name );
		exit( -1 );
	}

	uint64_t delta{ remote_module - local_module };

	return ( ( uint64_t )GetProcAddress( ( HMODULE )local_module, func ) + delta );
}

void utils::solve_imports( uint8_t* base, IMAGE_NT_HEADERS* nt_header, IMAGE_IMPORT_DESCRIPTOR* import_descriptor ) {
	char* module;
	while( ( module = ( char* )get_ptr_from_rva( ( DWORD64 )( import_descriptor->Name ), nt_header, ( PBYTE )base ) ) )
	{
		HMODULE local_module{ LoadLibrary( module ) };

		IMAGE_THUNK_DATA* thunk_data{ ( IMAGE_THUNK_DATA* )get_ptr_from_rva( ( DWORD64 )( import_descriptor->FirstThunk ), nt_header, ( PBYTE )base ) };

		while( thunk_data->u1.AddressOfData )
		{
			IMAGE_IMPORT_BY_NAME* iibn{ ( IMAGE_IMPORT_BY_NAME* )get_ptr_from_rva( ( DWORD64 )( ( thunk_data->u1.AddressOfData ) ), nt_header, ( PBYTE )base ) };
			thunk_data->u1.Function = ( uint64_t )( GetFuncAddress( module, ( char* )iibn->Name ) );
			thunk_data++;
		}

		import_descriptor++;
	}

	return;
}
