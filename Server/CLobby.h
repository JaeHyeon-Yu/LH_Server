#pragma once
#include "CPlayer.h"
#include "packet.h"
#define MAX_LOBBY 4	
#define MAX_PLAYER 4

#define IS_NOT_ROOM -1

class CLobby {
private:
	int m_users;
	int m_idx;
	int m_hostIdx;
	int m_state{ IS_NOT_ROOM };
	CPlayer m_player[MAX_PLAYER];

public:
	CLobby() = default;
	~CLobby() = default;

	int AddUser(const CPlayer& player);
	void SubUsers(const CPlayer& player);

	int GetUsers();
	void SetIdx(int idx);
	int GetIdx();
	void SetHost(const CPlayer& player);
	std::string GetHostID();
	bool Start();

	SC_PLAYER GetPlayer2Pack(int n);
	void ReadyPlayer(int num);
	bool IsHost(const CPlayer& player);
	bool IsAllReady();

	// Test Code
	void ShowAllPlayers() const {
		for (auto p : m_player) 
			p.ShowPlayerInfo();		
	}
};

