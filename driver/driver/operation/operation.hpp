#pragma once
#include "../stdafx/stdafx.hpp"

enum e_opcode : BYTE {
	NONE,
	TEST,
};

enum class e_side : BYTE {
	NONE,
	CLIENT,
	SERVER,
};

struct server_test {};
struct client_test {
	bool is_valid = false;
};

struct packet_base_t {
	e_opcode opcode;
	e_side side;

	union {
		union {
			server_test test;
		} server;

		union {
			client_test test;
		} client;
	};
};