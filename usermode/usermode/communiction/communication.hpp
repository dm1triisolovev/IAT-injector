#pragma once
#include "../operation/operation.hpp"

class c_communication {
public:
	static operation_callback init( std::string, std::string );
	static bool test_operation( operation_callback );
};