#include "communiction/communication.hpp"

#include "utils/utils.hpp"

#include "memory/memory.hpp"

std::uint8_t* raw_data = 0;
std::size_t data_size = 0;

void map_sections( uint64_t base, IMAGE_NT_HEADERS* nt_header ) {
	auto header{ IMAGE_FIRST_SECTION( nt_header ) };
	size_t virtual_size{ 0 };
	size_t bytes{ 0 };

	while( nt_header->FileHeader.NumberOfSections && ( bytes < nt_header->OptionalHeader.SizeOfImage ) )
	{
		memory->write_memory( base + header->VirtualAddress, ( uintptr_t )( raw_data + header->PointerToRawData ), header->SizeOfRawData );

		virtual_size = header->VirtualAddress;
		virtual_size = ( ++header )->VirtualAddress - virtual_size;

		bytes += virtual_size;
	}

	return;
}

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
			pid = utils::get_proc_id( "Deceit.exe" );
			if( pid )
				break;
		}
	}

	memory->operation_callback = operation;
	memory->process_id = pid;

	std::string current_path = std::filesystem::current_path( ).string( );
	std::string image_path = current_path + "\\Hello.dll";

	std::ifstream file( image_path.c_str( ), std::ios::binary | std::ios::ate );
	if( !file ) {
		printf( "[ Username ] dll not found\n" );
	}

	std::ifstream::pos_type pos{ file.tellg( ) };

	data_size = pos;
	raw_data = new std::uint8_t[data_size];

	if( !raw_data )
		return false;

	file.seekg( 0, std::ios::beg );
	file.read( ( char* )raw_data, data_size );

	file.close( );

	uint8_t dll_stub[] = { "\x51\x52\x55\x56\x53\x57\x41\x50\x41\x51\x41\x52\x41\x53\x41\x54\x41\x55\x41\x56\x41\x57\x48\xB8\xFF\x00\xDE\xAD\xBE\xEF\x00\xFF\x48\xBA\xFF\x00\xDE\xAD\xC0\xDE\x00\xFF\x48\x89\x10\x48\x31\xC0\x48\x31\xD2\x48\x83\xEC\x28\x48\xB9\xDE\xAD\xBE\xEF\xDE\xAD\xBE\xEF\x48\x31\xD2\x48\x83\xC2\x01\x48\xB8\xDE\xAD\xC0\xDE\xDE\xAD\xC0\xDE\xFF\xD0\x48\x83\xC4\x28\x41\x5F\x41\x5E\x41\x5D\x41\x5C\x41\x5B\x41\x5A\x41\x59\x41\x58\x5F\x5B\x5E\x5D\x5A\x59\x48\x31\xC0\xC3" };

	IMAGE_DOS_HEADER* dos_header{ ( IMAGE_DOS_HEADER* )raw_data };

	if( dos_header->e_magic != IMAGE_DOS_SIGNATURE ){
		printf( "[ Usermode ] Invalid PE\n" );
	}

	IMAGE_NT_HEADERS* nt_header{ ( IMAGE_NT_HEADERS* )( &raw_data[dos_header->e_lfanew] ) };

	if( nt_header->Signature != IMAGE_NT_SIGNATURE ) {
		printf( "[ Usermode ] Invalid NT header\n" );
	}

	uint64_t base{ memory->allocate( nt_header->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) };
	if( !base ) {
		printf( "[ Usermode ] Failed to allocate memory for local image\n" );
	}

	uint64_t stub_base{ memory->allocate( sizeof( dll_stub ), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) };

	if( !stub_base ) {
		printf( "[ Usermode ] Failed to allocate memory for shellcode\n" );
	}

	PIMAGE_IMPORT_DESCRIPTOR import_descriptor{ ( PIMAGE_IMPORT_DESCRIPTOR )utils::get_ptr_from_rva(
												( uint64_t )( nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress ),
												nt_header,
												raw_data ) };

	if( nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size ) {
		printf( "[ Usermode ] Fixing imports...\n" );
		utils::solve_imports( raw_data, nt_header, import_descriptor );
	}

	PIMAGE_BASE_RELOCATION base_relocation{ ( PIMAGE_BASE_RELOCATION )utils::get_ptr_from_rva(
																		nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress,
																		nt_header,
																		raw_data ) };

	if( nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size ) {
		printf("[ Usermode ] Fixing relocations...\n" );

		utils::solve_relocations( ( uint64_t )raw_data,
			base,
			nt_header,
			base_relocation,
			nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size );
	}

	uint64_t process_base = memory->get_module( L"Deceit.exe" );

	if( !process_base ) {
		printf( "\n[ Usermode ] Failed to get process base." );
		exit( -1 );
	}

	printf("[ Usermode ] Parsing imports...\n" );

	if( !utils::parse_imports( process_base ) ) {
		printf( "\n[ Usermode ] Failed to parse imports." );
		exit( -1 );
	}

	uint64_t iat_function_ptr{ memory->imports["TranslateMessage"] };

	if( !iat_function_ptr ) {
		printf( "[ Usermode ] Target import not found." );
		exit( -1 );
	}

	uint64_t orginal_function_addr{ memory->read<uint64_t>( iat_function_ptr ) };

	printf( "\n[ Usermode ] IAT pointer: %p\n", iat_function_ptr );

	*( uint64_t* )( dll_stub + 0x18 ) = iat_function_ptr;
	*( uint64_t* )( dll_stub + 0x22 ) = orginal_function_addr;

	memory->write_memory( base, ( uintptr_t )raw_data, nt_header->FileHeader.SizeOfOptionalHeader + sizeof( nt_header->FileHeader ) + sizeof( nt_header->Signature ) );

	printf( "\n[ Usermode ] Mapping sections...\n" );

	map_sections( base, nt_header );

	uint64_t entry_point{ ( uint64_t )base + nt_header->OptionalHeader.AddressOfEntryPoint };

	*( uint64_t* )( dll_stub + 0x39 ) = ( uint64_t )base;
	*( uint64_t* )( dll_stub + 0x4A ) = entry_point;

	printf( "\n[ Usermode ] Entry Point: %p\n", entry_point );

	memory->write_memory( stub_base, ( uintptr_t )dll_stub, sizeof( dll_stub ) );
	memory->protect( iat_function_ptr, sizeof( uint64_t ), PAGE_READWRITE );
	memory->write_memory( iat_function_ptr, ( uintptr_t )&stub_base, sizeof( uint64_t ) );

	Sleep( 3000 );

	if( iat_function_ptr != NULL )
	{
		memory->protect( iat_function_ptr, sizeof( uint64_t ), PAGE_READONLY );

		delete[] raw_data;

		printf( "\n[ Usermode ] Done!" );

		Sleep( 1500 );
	}

	std::cin.get( );
	return 0;
}