#include "CPathFinder.h"

list<Position> CPathFinder::GetPath(Position start, Position end) {
	// Initialize List
	auto oIter = openList.begin();
	while (openList.empty() == false) {
		delete[](*oIter);
		openList.erase(oIter);
		oIter = openList.begin();
	}
	auto cIter = closedList.begin();
	while (closedList.empty() == false) {
		delete[](*cIter);
		closedList.erase(cIter);
		cIter = closedList.begin();
	}
	pathList.clear();
	/*
	�޸𸮸� �Ҵ��Ϸ� �־���µ� clear�� �޸𸮸� ����־ ���� ���峪?
	�Ҵ�� �޸𸮸� ���� ��������� ��������
	*/

	direction[X_UP] = { 1,0,0 };
	direction[X_DOWN] = { -1,0,0 };
	direction[Y_UP] = { 0,1,0 };
	direction[Y_DOWN] = { 0,-1,0 };


	if (!Overlap_Start_End(start, end))
		return pathList;
	
	if (!IsOutMap(end)) {
		return pathList;
	}

	Node* parent = new Node(start, nullptr, 0, end);
	closedList.push_back(parent);

	Node* pathFind = PathFind(parent, end);


	while (pathFind->GetParent() != NULL){
		pathList.insert(pathList.begin(), pathFind->GetPosition());
		pathFind = pathFind->GetParent();
	}
	return pathList;
}

Node* CPathFinder::PathFind(Node* parent, Position end) {
	if (!Overlap_Start_End(parent->GetPosition(), end)) {

		return parent;
	}

	for (int i = 0; i < WAY_DIR; ++i) {
		Position childPos = parent->GetPosition() + direction[i];
		if (IsOutMap(childPos)) {
			// if (GetDistance(childPos, end) > parent->GetCostH()) continue;
			Node* newNode = new Node(childPos, parent, parent->GetCostG() + 1, end);
			IsOverlap(newNode);
		}
	}

	Node *solution = openList.front();

	for (auto ol : openList)
		if (solution->GetCostF() >= ol->GetCostF())
			solution = ol;
	/*
	���� ������ ���⼭ �ִ°ű��� ������µ�
	�ְ� ���� ���⼭�� �ڽ�Ʈ�θ� �˻��ϴϱ�
	������ �ڽ�Ʈ�� �Ÿ��θ� �Ǿ����ݾ�?
	���� ����ġ�� ������ �ȵǼ�
	��ο� �������� �ִ� ��ֹ��� ������ 
	�������� �ִ°� �ƴµ� ����ġ �˻翡�� �̸� ������� �ʾƼ� 
	�ְ� �ű�� ������ �Ѵ�.
	�׷��� F����� ���̱��� ����ϵ��� �ٽ� ¥���Ѵ�.
	*/
	openList.remove(solution);
	closedList.push_back(solution);

	if (closedList.size() > MAX_LIST_LEN) return parent;
	return PathFind(solution, end);
}

bool CPathFinder::IsOutMap(const Position& pos) {
	// ��ǥ�� �ӽ���ǥ(�� �̿ϼ�) ���
	if (pos.x < -1807 || pos.x> 1007) return false;
	if (pos.y < -1407 || pos.y> 1407) return false;
	int xIdx = (int)pos.x + X_SIDE;
	int yIdx = (int)pos.y + Y_SIDE;
	// z������ �̵��� �� �ִ� �ִ� ���̺��� ������
	// if (board[xIdx][yIdx]-20 > ((int)pos.z))
	// 	return false;
	if (board[xIdx][yIdx] > 300) return false;
	return true;
} 

bool CPathFinder::IsOverlap(Node* node) {
	for (auto cl : closedList)
		if (!Overlap_Start_End(cl->GetPosition(), node->GetPosition())) 
			return true;
		
	for (auto ol : openList)
		if (!Overlap_Start_End(ol->GetPosition(), node->GetPosition())) {
			printf("���� �����̿�\n");
			if (ol->GetCostF() > node->GetCostF()) {
				openList.remove(ol);
				openList.push_back(node);
				return true;
			}
		}
	openList.push_back(node);
	return true;
}