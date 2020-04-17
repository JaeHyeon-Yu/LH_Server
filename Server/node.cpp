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
	���⼭ ���̸� ����ؾ� �Ѵٶ�...
	�߰������� ����ؾ��ϴµ�
	�׷����� �Ÿ��δ� �������
	��ġ? �׷�����
	�ƴ� �ٵ� �� �տ� ��ֹ��� �ִµ� �װ� ������ �̵��ص� �Ȱ��ݾ� �Ÿ��� 1���̴ϱ� ����
	������ �̵��ص� 1���̴ϱ� ���̰� ���̳���
	���� �� �յ� ������ ���� �������̶����� ��������
	�׷��ϱ� ��� �������� ��������
	�̰� ����?
	�Ƹ� �׷��ž�

	���� �ٽ� ���ϱ�
	������ ��� �͸����� �ϰ��־� ������
	�������� ���µ� �������� �� ���°� �ƴ϶� ���� �����ʿ� �����ϱ� �ٽ� ���������� ���ž�
	�ٵ� �����ʿ��� ������ �Ÿ��� �־����ϱ� �ٽ� �������� �̵��� ��
	�̰� ��ӹݺ��Ǵ°ž�
	��� �͸����� �ϴ°���
	*/
}