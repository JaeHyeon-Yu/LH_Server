#pragma once

// define 
#define SERVERPORT 9000
#define BUFSIZE 512

enum Player_State {
	In_Lobby, In_Room, In_Game
};

struct Player {
	int state;
	int idx;
};

// struct
struct SOCKETINFO {
	OVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE];
	int recvBytes;
	int sendBytes;
	WSABUF wsabuf;
};