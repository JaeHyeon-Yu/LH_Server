#pragma once

#define MAXLEN 10

enum PACKET_TYPE {
	login_packet, signup_packet, lobby_packet, player_packet
};

enum Login_State {
	login_ok, login_error,
	signup_ok, signup_error,
	join_ok, join_error,
	start_ok, start_fail
};

enum Lobby_State {
	create_room, join_room, update_room, out_room, update_player,
	ready, start
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
struct SC_LOBBY {
	char type;
	char state;
	int idx;
	int users;
};
struct CS_LOBBY {
	char type;
	char state;
	int num;
};
struct SC_PLAYER {
	char type;
	int idx;
	char id[MAXLEN];
	char state;
};
#pragma pack(pop)