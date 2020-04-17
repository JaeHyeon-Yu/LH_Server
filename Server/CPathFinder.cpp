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
	메모리를 할당하려 넣어줬는데 clear로 메모리를 비워주어서 에러 생겼나?
	할당된 메모리를 각각 명시적으로 삭제해줌
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
	지금 문제가 저기서 넣는거까진 상관없는데
	넣고 나서 여기서는 코스트로만 검사하니까
	지금은 코스트가 거리로만 되어있잖아?
	높이 가중치가 적용이 안되서
	경로에 높이차가 있는 장애물이 있으면 
	높이차가 있는건 아는데 가중치 검사에서 이를 고려하지 않아서 
	애가 거기로 가려고만 한다.
	그러면 F비용을 높이까지 고려하도록 다시 짜야한다.
	*/
	openList.remove(solution);
	closedList.push_back(solution);

	if (closedList.size() > MAX_LIST_LEN) return parent;
	return PathFind(solution, end);
}

bool CPathFinder::IsOutMap(const Position& pos) {
	// 좌표는 임시좌표(맵 미완성) 사용
	if (pos.x < -1807 || pos.x> 1007) return false;
	if (pos.y < -1407 || pos.y> 1407) return false;
	int xIdx = (int)pos.x + X_SIDE;
	int yIdx = (int)pos.y + Y_SIDE;
	// z축으로 이동할 수 있는 최대 높이보다 높은가
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
			printf("나는 개인이오\n");
			if (ol->GetCostF() > node->GetCostF()) {
				openList.remove(ol);
				openList.push_back(node);
				return true;
			}
		}
	openList.push_back(node);
	return true;
}