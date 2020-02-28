#include "CLobby.h"
int CLobby::AddUser(const CPlayer& player) { 
	// Add User to Room (upper 4) and return Player's index
	int idx;
	for (int i = 0; i<MAX_PLAYER; ++i)
		if (m_player[i].GetState() == not_login) {
			m_player[i] = player;
			idx = i;
			m_player[i].ShowPlayerInfo();
			break;
		}
	++m_users; 
	return idx;
}
void CLobby::SubUsers(const CPlayer& player) {
	for (int i = 0; i < MAX_PLAYER; ++i) {
		if (m_player[i] == player) {
			m_player[i].OutLobby();
	
			if (i == m_hostIdx)
				// If no user without host, delete room 
				if (m_users <= 1) {
					printf("%d room is delete (no Users)\n", m_idx);
					m_idx = IS_NOT_ROOM;
				}
				// If another user, host change 
				else 
					for (int j = 0; j < MAX_PLAYER; ++j)
						if (m_player[j].GetState() != not_login) {
							m_hostIdx = j;
							printf("%d room host is changed - %s\n", m_idx, m_player[m_hostIdx].GetID().c_str());
							break;
						}
		}
	}
	--m_users;
}
SC_PLAYER CLobby::GetPlayer2Pack(int n) {
	// m_player 's index : n (0~3)을 입력받아 해당하는 플레이어의 패킷을 만들어준다.
	SC_PLAYER player;
	player.type = player_packet;
	if (m_player[n].GetState() == not_login) return SC_PLAYER();
	player.idx = n;
	if (n == m_hostIdx) player.state = host;
	else if (m_player[n].GetState() == ready) player.state = ready;
	else if (m_player[n].GetState() == start) player.state = start;
	else player.state = none_ready;
	strcpy_s(player.id, m_player[n].GetID().c_str());
	m_player[n].ShowPlayerInfo();
	return player;
}
void CLobby::ReadyPlayer(int num) {
	if (num == m_hostIdx) return;
	m_player[num].Ready();
}
bool CLobby::IsHost(const CPlayer& player) {
	return m_player[m_hostIdx] == player;
}
bool CLobby::IsAllReady() {
	for (int i = 0; i < MAX_PLAYER; ++i)
		if (m_player[i].GetState() == not_login || i == m_hostIdx) continue;
		else if (m_player[i].GetState() != ready) return false;
	return true;
}
bool CLobby::Start() {
	if (IsAllReady() == false) {
		printf("%d room is start fail - host:%s\n", m_idx, m_player[m_hostIdx].GetID().c_str());
		return false;
	}
	for (int i = 0; i < MAX_PLAYER; ++i)
		m_player[i].SetState(start);

	printf("%d room is start ok - host:%s\n", m_idx, m_player[m_hostIdx].GetID().c_str());
	return true;
}


// Getter & Setter
int CLobby::GetUsers() {
	return m_users;
}
void CLobby::SetIdx(int idx) {
	m_idx = idx;
}
int CLobby::GetIdx() {
	return m_idx;
}
void CLobby::SetHost(const CPlayer& player) {
	for (int i = 0; i < MAX_PLAYER; ++i)
		if (m_player[i] == player) {
			m_hostIdx = i;
			break;
		}
}
std::string CLobby::GetHostID() {
	return m_player[m_hostIdx].GetID();
}
