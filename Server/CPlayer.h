#pragma once
#include <string>
#include <iostream>
#define MAX_CLIENTS 100

enum Player_State {
	not_login = -1, In_Lobby, In_Room, In_Game,
	host, none_ready
};

class CPlayer {
private:
	std::string m_id;
	std::string m_pass;
	int m_state{not_login};
	int m_level;

	int m_roomNum;
	int m_idx;
	bool m_ready{ false };
	bool m_host{ false };
public:
	CPlayer() = default;
	CPlayer(std::string id, std::string pass);
	~CPlayer() = default;

	void Initialize(const CPlayer& p);

	// overloading
	bool operator== (const CPlayer& cp) const;

	// Getter & Setter
	void SetState(const int& state);
	int GetState();
	std::string GetID();
	int GetRoomNum();
	bool GetHost() { return m_host; }
	void SetIdx(int n) { m_idx = n; }
	int GetIdx() { return m_idx; }
	// ??
	void Ready();

	// TestCode
	void ShowPlayerInfo() const {
		std::cout <<"player - "<< m_id << "\t" << m_pass << std::endl;
	}
	void OutLobby() {
		m_state = not_login;
		m_ready = false;
		m_host = false;
		m_id.clear();
		m_pass.clear();
	}
};

