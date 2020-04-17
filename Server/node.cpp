#include "node.h"

bool Overlap_Start_End(Position s, Position e) {
	if (s == e) return true;
	return false;
}

float GetDistance(Position p, Position e) {
	float x = pow((p.x - e.x), 2);
	float y = pow((p.y - e.y), 2);
	float z = pow((p.z - e.z), 2);
	float distance = sqrt(x + y + z);
	return abs(distance);
}

Node::Node(Position pos, Node* parent, int g, Position h) {
	currentPos = pos;
	// currentPos.z = board[X_SIDE][Y_SIDE];
	this->parent = parent;
	costG = g;
	SetCostH(h);
}

int Node::GetCostG() const {
	return costG;
}

float Node::GetCostH() const {
	return costH;
}

float Node::GetCostF() const {
	return ((float)costG + costH);
}

Node* Node::GetParent() const {
	return parent;
}

Position Node::GetPosition() const {
	return currentPos;
}

void Node::SetCostG(const int& g) {
	costG = g;
}

void Node::SetCostH(Position p) {
	costH = GetDistance(currentPos, p);
	
	/*
	여기서 높이를 고려해야 한다라...
	중간값들을 고려해야하는데
	그러려먼 거리로는 곤란하지
	그치? 그러면은
	아니 근데 이 앞에 장애물이 있는데 그걸 옆으로 이동해도 똑같잖아 거리가 1차이니까 고작
	옆으로 이동해도 1차이니까 높이가 차이나면
	옆이 더 먼데 심지어 옆도 높이차이때문에 못움직여
	그러니까 계속 직선으로 가려고해
	이게 맞지?
	아마 그럴거야

	지금 다시 보니까
	옆으로 계속 와리가리 하고있어 막히면
	왼쪽으로 갔는데 왼쪽으로 더 가는게 아니라 내가 오른쪽에 있으니까 다시 오른쪽으로 간거야
	근데 오른쪽에선 막히고 거리도 멀어지니까 다시 왼쪽으로 이동을 해
	이게 계속반복되는거야
	계속 와리가리 하는거지
	*/
}