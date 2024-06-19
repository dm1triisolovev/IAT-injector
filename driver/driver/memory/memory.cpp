#include "memory.hpp"

NTSTATUS c_memory::copy_memory( uint32_t current_proc_id, uint32_t target_proc_id, uint64_t current_dest_addr, uint64_t target_dest_address, uint32_t mem_size ) {
    NTSTATUS status = STATUS_SUCCESS;

    PEPROCESS target_process;
    status = PsLookupProcessByProcessId( ( HANDLE )target_proc_id, &target_process );

    if( !NT_SUCCESS( status ) )
        return STATUS_INVALID_CID;

    PEPROCESS current_process;
    status = PsLookupProcessByProcessId( ( HANDLE )current_proc_id, &current_process );

    if( !NT_SUCCESS( status ) ) {
        ObDereferenceObject( current_process );
        return STATUS_INVALID_CID;
    }

    SIZE_T return_size = 0;
    
    status = MmCopyVirtualMemory( current_process, ( void* )current_dest_addr, target_process, ( void* )target_dest_address, mem_size, KernelMode, &return_size );

    ObDereferenceObject( target_process );
    ObDereferenceObject( current_process );

    return status;
}

NTSTATUS c_memory::allocate_virtual_memory( uint32_t target_proc_id, PVOID* base_address, SIZE_T size, ULONG allocation_type, ULONG protect ) {
    NTSTATUS status = STATUS_SUCCESS;

    PEPROCESS target_process;
    status = PsLookupProcessByProcessId( ( HANDLE )target_proc_id, &target_process );

    if( !NT_SUCCESS( status ) )
        return STATUS_INVALID_CID;

    KAPC_STATE apc_state;
    KeStackAttachProcess( ( PRKPROCESS )target_process, &apc_state );

    status = ZwAllocateVirtualMemory( ZwCurrentProcess( ), base_address, 0, &size, allocation_type, protect );

    KeUnstackDetachProcess( &apc_state );
    ObDereferenceObject( target_process );

    return status;
}

NTSTATUS c_memory::protect_virtual_memory( uint32_t target_proc_id, PVOID base_address, SIZE_T size, ULONG new_protect ) {
    NTSTATUS status = STATUS_SUCCESS;

    PEPROCESS target_process;
    status = PsLookupProcessByProcessId( ( HANDLE )target_proc_id, &target_process );

    if( !NT_SUCCESS( status ) )
        return STATUS_INVALID_CID;

    KAPC_STATE apc_state;
    KeStackAttachProcess( ( PRKPROCESS )target_process, &apc_state );

    ULONG old_protect = 0;

    status = ZwProtectVirtualMemory( ZwCurrentProcess( ), &base_address, &size, new_protect, &old_protect );

    KeUnstackDetachProcess( &apc_state );

    new_protect = old_protect;

    ObDereferenceObject( target_process );

    return status;
}

NTSTATUS c_memory::free_virtual_memory( uint32_t target_proc_id, PVOID base_address ) {
    NTSTATUS status = STATUS_SUCCESS;

    PEPROCESS target_process;
    status = PsLookupProcessByProcessId( ( HANDLE )target_proc_id, &target_process );

    if( !NT_SUCCESS( status ) )
        return STATUS_INVALID_CID;

    KAPC_STATE apc_state;
    KeStackAttachProcess( ( PRKPROCESS )target_process, &apc_state );

    SIZE_T region_size = 0;

    status = ZwFreeVirtualMemory( ZwCurrentProcess( ), &base_address, &region_size, MEM_RELEASE );

    KeUnstackDetachProcess( &apc_state );
    ObDereferenceObject( target_process );

    return status;
}