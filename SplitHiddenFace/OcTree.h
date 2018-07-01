#pragma once
struct Object {
	Point center;
	std::vector<FbxVector4> vertices;        // Radius of object bounding sphere
	Object *pNextObject; // Pointer to next object when linked into list
	unsigned objectID;
	Object(Point center, std::vector<FbxVector4> vertices, unsigned objectID) :center(center), vertices(vertices), objectID(objectID) {};
};

struct Node {
	Point center;     //
	double halfWidth;  // Half the width of the node volume (not strictly needed)
	Node *pChild[8];  // Pointers to the eight children nodes
	Object *pObjList;
};

//创建空八叉树，里面没有数据，需要调用insertObject充实
Node *buildEmptyOctree(Point center, double halfWidth, int stopDepth);

//一个小函数，被insertObject调用
int celGreaterThanZero(double v);

//在八叉树插入数据（对象）
void insertObject(Node *root, Object *pObject);

//销毁八叉树
void destoryTree(Node* root);