#include "stdafx.h"

Node *buildEmptyOctree(Point center, double halfWidth, int stopDepth)
{
	if (stopDepth < 0)
		return NULL;
	else
	{
		//创建root结点
		Node *pNode = new Node;
		pNode->center = center;
		pNode->halfWidth = halfWidth;
		pNode->pObjList = NULL;

		// 递归产生root结点的8个孩子结点
		Point _offset;
		double step = halfWidth * 0.5f;
		for (int i = 0; i < 8; i++) {
			_offset.x = ((i & 1) ? step : -step);
			_offset.y = ((i & 2) ? step : -step);
			_offset.z = ((i & 4) ? step : -step);
			pNode->pChild[i] = buildEmptyOctree(center + _offset, step, stopDepth - 1);
		}
		return pNode;
	}
}

int celGreaterThanZero(double v)
{
	if (v > 0.0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void insertObject(Node *root, Object *pObject)
{
	bool straddle = false;
	unsigned vertexCount = pObject->vertices.size();
	for (unsigned i = 0; i < vertexCount; ++i)
	{
		double delta_x = abs(root->center.x - pObject->vertices[i][0]);
		double delta_y = abs(root->center.y - pObject->vertices[i][1]);
		double delta_z = abs(root->center.z - pObject->vertices[i][2]);
		double maxDelta = delta_x > delta_y ? delta_x : delta_y;
		maxDelta = maxDelta > delta_z ? maxDelta : delta_z;
		if (maxDelta > root->halfWidth)
		{
			straddle = true;
			break;
		}
	}
	if (!straddle)
	{
		FbxVector4 center = pObject->vertices[0];
		int index = 0;
		double delta_x = pObject->center.x - root->center.x;
		double delta_y = pObject->center.y - root->center.y;
		double delta_z = pObject->center.z - root->center.z;
		index = celGreaterThanZero(delta_z) * 4 + celGreaterThanZero(delta_y) * 2 + celGreaterThanZero(delta_x);
		if (root->pChild[index] == NULL)
		{
			pObject->pNextObject = root->pObjList;
			root->pObjList = pObject;
		}
		else
		{
			insertObject(root->pChild[index], pObject);
		}
	}
	else
	{
		pObject->pNextObject = root->pObjList;
		root->pObjList = pObject;
	}
}

void destoryTree(Node* root)
{
	if (root == nullptr)
	{
		return;
	}
	for (int i = 0; i < 8; ++i)
	{
		destoryTree(root->pChild[i]);
	}
	while (root->pObjList)
	{
		Object *p = root->pObjList;
		root->pObjList = p->pNextObject;
		delete p;
	}
	delete root;
}