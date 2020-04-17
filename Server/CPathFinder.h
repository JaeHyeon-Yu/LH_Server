#pragma once
#include "node.h"
#define WAY_DIR 4
#define X_UP 0
#define X_DOWN 1
#define Y_UP 2
#define Y_DOWN 3


#define MAX_LIST_LEN 1000

class CPathFinder {
private:
	list<Node*> openList;
	list<Node*> closedList; 
	list<Position> pathList;
	Position start, end;
	Position direction[WAY_DIR];
	// Position current;
	int idx;
public:
	CPathFinder() = default;
	~CPathFinder() = default;

	bool IsOutMap(const Position& pos);
	bool IsOverlap(Node* node);
	list<Position> GetPath(Position start, Position end);
	Node* PathFind(Node* parent, Position end);
};

