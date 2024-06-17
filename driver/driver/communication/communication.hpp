#pragma once

#include "../operation/operation.hpp"

using fn_operation_callback = PVOID( __fastcall* )( packet_base_t&, PBYTE );
extern fn_operation_callback o_operation_callback;

class c_communication {
public:
	static PVOID operation_callback( packet_base_t&, PBYTE );
	static NTSTATUS setup();
};