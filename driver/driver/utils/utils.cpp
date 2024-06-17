#include "utils.hpp"

bool data_compare( const char* data, const char* mask, const char* sz_mask ) {
	for( ; *sz_mask; ++sz_mask, ++data, ++mask )
	{
		if( *sz_mask == ( "x" )[0] && *data != *mask )
			return false;
	}

	return !*sz_mask;
}

uintptr_t utils::find_pattern( const uintptr_t base, const size_t size, const char* mask, const char* sz_mask )
{
	for( size_t i = 0; i < size; ++i )
		if( data_compare( reinterpret_cast< const char* >( base + i ), mask, sz_mask ) )
			return base + i;

	return 0;
}

FORCEINLINE PVOID kernel_alloc( ULONG size ) {
	PVOID buffer = ExAllocatePoolWithTag( NonPagedPool, size, 'BOB3' );
	__stosb( ( PUCHAR )buffer, 0, size );

	return buffer;
}

PVOID query_sys_inforamtion( SYSTEM_INFORMATION_CLASS _class ) {
query: ULONG query_size = 0;
	ZwQuerySystemInformation( _class, nullptr, query_size, &query_size );

	if( !query_size )
		goto query;

	PVOID info = kernel_alloc( query_size );

	if( !NT_SUCCESS( ZwQuerySystemInformation( _class, info, query_size, &query_size ) ) ) {
		ExFreePoolWithTag( info, 'BOB3' );
		goto query;
	}

	return info;
}

template <typename str_type, typename str_type_2>
FORCEINLINE bool str_cmp( str_type str, str_type_2 in_str, bool two ) {
#define ToLower(_char) ((_char >= 'A' && _char <= 'Z') ? (_char + 32) : _char)

	if( !str || !in_str )
		return false;

	wchar_t c1, c2; do
	{
		c1 = *str++; c2 = *in_str++;
		c1 = ToLower( c1 ); c2 = ToLower( c2 );

		if( !c1 && ( two ? !c2 : 1 ) )
			return true;

	} while( c1 == c2 );

	return false;
}

uintptr_t utils::get_system_base( LPCSTR module_name, PULONG module_size ) {
	PRTL_PROCESS_MODULES module_list = ( PRTL_PROCESS_MODULES )query_sys_inforamtion( SystemModuleInformation );

	if( !module_list ) {
		DbgPrint( "[ Drvier ] Failed to get module list.\n" );
		return 0;
	}

	uintptr_t kernel_base = 0;

	if( module_list->NumberOfModules > 0 ) {
		if( !module_name ) {
			kernel_base = ( uintptr_t )module_list->Modules[0].ImageBase;
			if( module_size )
				*module_size = module_list->Modules[0].ImageSize;
		}
		else {
			for( ULONG i = 0; i < module_list->NumberOfModules; i++ )
			{
				RTL_PROCESS_MODULE_INFORMATION kernel_module = module_list->Modules[i];

				if( str_cmp( &kernel_module.FullPathName[kernel_module.OffsetToFileName], module_name, true ) )
				{
					kernel_base = ( uintptr_t )kernel_module.ImageBase;

					if( module_size )
						*module_size = kernel_module.ImageSize;

					break;
				}
			}
		}
	}

	ExFreePoolWithTag( module_list, 'BOB3' );

	return kernel_base;
}