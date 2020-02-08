#pragma once

#define MAXLEN 10

enum PACKET_TYPE {
	login_packet, signup_packet
};

enum Login_State {
	login_ok, login_error,
	signup_ok, signup_error
};

#pragma pack(push, 1)
struct CS_LOGIN {
	char type;
	char id[MAXLEN];
	char password[MAXLEN];
};
struct SC_LOGIN {
	char type;
	char state;
};
struct CS_SIGNUP {
	char type;
	char id[MAXLEN];
	char password[MAXLEN];
};
struct SC_SIGNUP {
	char type;
	char state;
};
#pragma pack(pop)