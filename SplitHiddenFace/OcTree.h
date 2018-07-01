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

//�����հ˲���������û�����ݣ���Ҫ����insertObject��ʵ
Node *buildEmptyOctree(Point center, double halfWidth, int stopDepth);

//һ��С��������insertObject����
int celGreaterThanZero(double v);

//�ڰ˲����������ݣ�����
void insertObject(Node *root, Object *pObject);

//���ٰ˲���
void destoryTree(Node* root);