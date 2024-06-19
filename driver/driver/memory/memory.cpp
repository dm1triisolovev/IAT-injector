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
