#include "stdafx.h"

enum PointToPolygon
{
	POINT_INFRONT_POLYGON = 1,
	POINT_BEHIND_POLYGON = 2,
	POINT_ON_POLYGON = 3
};

std::vector<unsigned>offSet;
std::vector<Point>prePoints;
std::map<int, std::vector<Shape>>additionPolygon;
std::map<int, std::vector<int>>collisionTable;

void convertMeshPolygonToShape(std::vector<FbxMesh*>meshVector, std::vector<Shape>&shapes)
{
	unsigned numMesh = meshVector.size();
	for (unsigned i = 0; i < numMesh; ++i)
	{
		unsigned off_set = offSet[i];
		FbxVector4* controlPointsVertex = meshVector[i]->GetControlPoints();
		int controlPointCount = meshVector[i]->GetControlPointsCount();
		int polygonCountInMesh_I = meshVector[i]->GetPolygonCount();
		for (int pPolygonIndex = 0; pPolygonIndex < polygonCountInMesh_I; ++pPolygonIndex)
		{
			int polygonVertexCount = meshVector[i]->GetPolygonSize(pPolygonIndex);
			std::vector<Point> shape(polygonVertexCount);
			std::vector<int> vertexIndex(polygonVertexCount);
			for (int pPositionInPolygon = 0; pPositionInPolygon < polygonVertexCount; ++pPositionInPolygon)
			{
				int _vi = meshVector[i]->GetPolygonVertex(pPolygonIndex, pPositionInPolygon);
				Point p(controlPointsVertex[_vi]);
				shape[pPositionInPolygon] = p;
				vertexIndex[pPositionInPolygon] = _vi;
			}
			FbxVector4 normal;
			Point ab = shape[1] - shape[0];
			Point ac = shape[2] - shape[0];
			Point n = ab.cross(ac);
			normal[0] = n.x;
			normal[1] = n.y;
			normal[2] = n.z;
			normalize(normal);
			shapes[off_set + pPolygonIndex].set(shape, normal, vertexIndex, shape.size());
		}
	}
}

bool neighborShape(Shape A, Shape B)
{
	for (unsigned i = 0; i < A.size(); ++i)
	{
		for (unsigned j = 0; j < B.size(); ++j)
		{
			if (A.getVertexAt(i) == B.getVertexAt(j))
			{
				return true;
			}
		}
	}
	return false;
}

int classifyPointToPlane(Point p, Shape shape)
{
	Point _normal = shape.getNormal();
	double dist = p.dot(_normal) - shape[1].dot(_normal);
	if (dist > PLANETHICNESS)
	{
		return POINT_INFRONT_POLYGON;
	}
	else if (dist < -PLANETHICNESS)
	{
		return POINT_BEHIND_POLYGON;
	}
	else
	{
		return POINT_ON_POLYGON;
	}
}

bool PointInsideTriangle(Point a, Point b, Point c, Point p)
{
	a = a - p;
	b = b - p;
	c = c - p;
	if (absoluteValue(a) < POINTDISTANCE || absoluteValue(b) < POINTDISTANCE || absoluteValue(c) < POINTDISTANCE)
	{
		return true;
	}
	double ab = a.dot(b);
	double ac = a.dot(c);
	double bc = b.dot(c);
	double cc = c.dot(c);
	if (bc*ac - cc * ab < -PLANETHICNESS)
	{
		return false;
	}
	double bb = b.dot(b);
	if (ab*bc - ac * bb < -PLANETHICNESS)
	{
		return false;
	}
	return true;
}

bool PointInsidePolygon(Point returnPoint, Shape collisionFace)
{
	unsigned collisionFaceSize = collisionFace.size();
	for (unsigned i = 1; i < collisionFaceSize - 1; ++i)
	{
		if (PointInsideTriangle(collisionFace[0], collisionFace[i], collisionFace[i + 1], returnPoint))
		{
			return true;
		}
	}
	return false;
}

bool intersectSegmentPolygon(Point startPoint, Point endPoint, Point& p, Shape collisionFace, Point prePoint)
{
	Point v = endPoint - startPoint;
	//(startPoint+v*t)·normal = collisionFace[0]·normal
	//collisionFace[0]·normal-startPoint·normal=v*t·normal
	Point _normal = collisionFace.getNormal();
	//存在精确误差，认为在一个epsilon范围内都属于t的值
	double _t = (collisionFace[0].dot(_normal) - startPoint.dot(_normal)) / (v.dot(_normal));
	double t[3] = { _t,_t + PLANETHICNESS / v.dot(_normal),_t - PLANETHICNESS / v.dot(_normal) };
	Point returnPoint;
	for (unsigned i = 0; i < 3; ++i)
	{
		returnPoint.x = startPoint.x + v.x*t[i];
		returnPoint.y = startPoint.y + v.y*t[i];
		returnPoint.z = startPoint.z + v.z*t[i];
		if (PointInsidePolygon(returnPoint, collisionFace))
		{
			if (!returnPoint.equal(prePoint))
			{
				p = returnPoint;
				return true;
			}
		}
	}
	return false;
}

bool findInnerPointInDetecFace(Shape detectFace, Shape collisionFace, Point& _p, Point prePoint)
{
	unsigned collisionFaceSize = collisionFace.size();
	for (unsigned i = 0; i < collisionFaceSize; ++i)
	{
		if (classifyPointToPlane(collisionFace[i], detectFace) == POINT_ON_POLYGON)
		{
			if (PointInsidePolygon(collisionFace[i], detectFace))
			{
				if (!collisionFace[i].equal(prePoint))
				{
					_p = collisionFace[i];
					return true;
				}
			}
		}
	}
	return false;
}

bool _update(unsigned& controlPointIndex, Point startPoint, Point endPoint, Point _p, Shape collisionFace, Shape detectFace, Shape& _shape)
{
	Point prePoint;
	if (prePoints.size() > 0)
	{
		prePoint = prePoints[prePoints.size() - 1];
	}
	if (intersectSegmentPolygon(startPoint, endPoint, _p, collisionFace, prePoint))
	{
		_shape.insert(_p, controlPointIndex++);
		prePoints.push_back(_p);
		return true;
	}
	else if (findInnerPointInDetecFace(detectFace, collisionFace, _p, prePoint))
	{
		_shape.insert(_p, controlPointIndex++);
		prePoints.push_back(_p);
		return true;
	}
	return false;
}

bool updateShapeByOneCollisionFace(FbxMesh* lMesh, Shape& detectFace, Shape collisionFace)
{
	Shape _shape;
	unsigned detectFaceSize = detectFace.size();
	std::vector<int>intersect(detectFaceSize);
	std::vector<int>pointPosition(4, 0);
	unsigned controlPointIndex = lMesh->GetControlPointsCount(), tempControlPointIndex = controlPointIndex;//需要记录点在mesh中的位置
	prePoints.clear();//清空所有数据
	for (unsigned i = 0; i < detectFaceSize; ++i)
	{
		intersect[i] = classifyPointToPlane(detectFace[i], collisionFace);
		++pointPosition[intersect[i]];
	}
	//detectFace全在collisionFace一侧或是在collisionFace面上时
	if (pointPosition[POINT_ON_POLYGON] != 0 || (pointPosition[POINT_BEHIND_POLYGON] & pointPosition[POINT_INFRONT_POLYGON]) == 0)
	{
		return false;
	}
	//只处理简单情况，不考虑复杂的
	for (unsigned i = 0; i < detectFaceSize; ++i)
	{
		if (intersect[i] == POINT_BEHIND_POLYGON)
		{
			Point startPoint, endPoint, _p;
			if (intersect[(i - 1) % detectFaceSize] == POINT_INFRONT_POLYGON)
			{
				startPoint = detectFace[(i - 1) % detectFaceSize];
				endPoint = detectFace[i];
				if (!_update(tempControlPointIndex, startPoint, endPoint, _p, collisionFace, detectFace, _shape))
				{
					return false;
				}
			}
			if (intersect[(i + 1) % detectFaceSize] == POINT_INFRONT_POLYGON)
			{
				endPoint = detectFace[(i + 1) % detectFaceSize];
				startPoint = detectFace[i];
				if (!_update(tempControlPointIndex, startPoint, endPoint, _p, collisionFace, detectFace, _shape))
				{
					return false;
				}
			}
		}
		else
		{
			_shape.insert(detectFace[i], detectFace.getVertexAt(i));
		}
	}
	//将产生的分割点保存到mesh中
	for (unsigned i = controlPointIndex; i < tempControlPointIndex; ++i)
	{
		FbxVector4 newPoint;
		for (unsigned j = 0; j < _shape.size(); ++j)
		{
			if (i == _shape.getVertexAt(j))
			{
				newPoint[0] = _shape[j].x;
				newPoint[1] = _shape[j].y;
				newPoint[2] = _shape[j].z;
				break;
			}
		}
		lMesh->SetControlPointAt(newPoint, i);
	}
	_shape.setNormal(detectFace.getNormal());
	detectFace = _shape;
	return true;
}

Node* buildOcTree(std::vector<FbxMesh*>&meshVector)
{
	//创建八叉树写死了，后续应该要根据模型的自适应调整
	Point rootTreeCenter(-100, 0, -100);
	Node*root = buildEmptyOctree(rootTreeCenter, 200, 2);
	for (unsigned i = 0; i < meshVector.size(); ++i)
	{
		FbxVector4* controlPoints = meshVector[i]->GetControlPoints();
		int polygonCount = meshVector[i]->GetPolygonCount();
		for (int j = 0; j < polygonCount; ++j)
		{
			int polygonVerticeCount = meshVector[i]->GetPolygonSize(j);
			std::vector<FbxVector4>vertices(polygonVerticeCount);
			double sum_x = 0.0, sum_y = 0.0, sum_z = 0.0;
			for (int vertexIndex = 0; vertexIndex < polygonVerticeCount; ++vertexIndex)
			{
				vertices[vertexIndex] = controlPoints[meshVector[i]->GetPolygonVertex(j, vertexIndex)];
				sum_x += vertices[vertexIndex][0];
				sum_y += vertices[vertexIndex][1];
				sum_z += vertices[vertexIndex][2];
			}
			Point center(sum_x / polygonVerticeCount, sum_y / polygonVerticeCount, sum_z / polygonVerticeCount);
			Object* _object = new Object(center, vertices, offSet[i] + j);
			insertObject(root, _object);
		}
	}
	return root;
}

void testAllCollisions(Node *root, std::vector<Shape>&shapes)
{
	const int MAX_DEPTH = 10;
	static Node *ancestorStack[MAX_DEPTH];
	static int depth = 0;
	ancestorStack[depth++] = root;
	for (int n = 0; n < depth; ++n) {
		Object *objectA, *objectB;
		for (objectA = ancestorStack[n]->pObjList; objectA; objectA = objectA->pNextObject)
		{
			Shape shape = shapes[objectA->objectID].expansion();
			for (objectB = root->pObjList; objectB; objectB = objectB->pNextObject)
			{
				if (objectA == objectB)
				{
					continue;
				}
				int objectAID = objectA->objectID, objectBID = objectB->objectID;
				if (gjk(shape, shapes[objectBID]))
				{
					if (!neighborShape(shapes[objectAID], shapes[objectBID]))//point在边上
					{
						if (collisionTable.find(objectAID) == collisionTable.end())
						{
							std::vector<int>_vector;
							_vector.push_back(objectBID);
							collisionTable.insert(make_pair(objectAID, _vector));
						}
						else
						{
							collisionTable[objectAID].push_back(objectBID);
						}
					}
				}
			}
		}
	}
	for (int i = 0; i < 8; i++)
	{
		if (root->pChild[i])
		{
			testAllCollisions(root->pChild[i], shapes);
		}
	}
	depth--;
}

void splitShapeInfo(std::vector<FbxMesh*>&meshVector, std::vector<Shape>&shapes, std::vector<int>&isChanged)
{
	Node* root = buildOcTree(meshVector);
	testAllCollisions(root, shapes);
	for (auto iter = collisionTable.begin(); iter != collisionTable.end(); ++iter)
	{
		unsigned detectFaceID = iter->first;
		unsigned collisionFaceCount = iter->second.size();
		int meshID;
		for (unsigned m = meshVector.size() - 1; m >= 0; --m)
		{
			if (detectFaceID >= offSet[m])
			{
				meshID = m;
				break;
			}
		}
		if (collisionFaceCount == 0)//说明没有相交
		{
			continue;
		}
		else if (collisionFaceCount == 1)//说明只有一个面相交，考虑两种情况
		{
			if (updateShapeByOneCollisionFace(meshVector[meshID], shapes[detectFaceID], shapes[iter->second[0]]))
			{
				isChanged[detectFaceID] = 1;
				std::vector<Shape>vp;
				vp.push_back(shapes[detectFaceID]);
				additionPolygon.insert(std::make_pair(detectFaceID, vp));
			}
		}
		else//与多个面相交，采取另外一种措施
		{
			for (unsigned onShapeIndex = 0; onShapeIndex < collisionFaceCount; ++onShapeIndex)
			{
				Shape _shape = shapes[detectFaceID];
				if (updateShapeByOneCollisionFace(meshVector[meshID], _shape, shapes[iter->second[onShapeIndex]]))
				{
					isChanged[detectFaceID] = 1;
					if (additionPolygon.find(detectFaceID) == additionPolygon.end())
					{
						std::vector<Shape>vp;
						vp.push_back(_shape);
						additionPolygon.insert(std::make_pair(detectFaceID, vp));
					}
					else
					{
						additionPolygon[detectFaceID].push_back(_shape);
					}
				}
			}
		}
	}
	destoryTree(root);
}

void removeHiddenShape(std::vector<FbxMesh*>meshVector, std::vector<Shape>shapes, std::vector<int>isChanged, int numPolygon)
{
	unsigned numMesh = meshVector.size();
	for (int i = numPolygon - 1; i >= 0; --i)
	{
		if (isChanged[i] == 1)
		{
			int meshID, polygonID;
			for (int j = numMesh - 1; j >= 0; --j)
			{
				if (i >= offSet[j])
				{
					meshID = j;
					polygonID = i - offSet[j];
					break;
				}
			}
			if (additionPolygon.find(i) == additionPolygon.end())//没有多余的多边形添加
			{
				Shape _shape = shapes[i];
				meshVector[meshID]->BeginPolygon();
				for (int j = 0; j < _shape.size(); ++j)
				{
					meshVector[meshID]->AddPolygon(_shape.getVertexAt(j));
				}
				meshVector[meshID]->EndPolygon();
			}

			else//可能要添加好几个多边形
			{
				std::vector<Shape>_polygon = additionPolygon[i];
				for (int it = 0; it < _polygon.size(); ++it)
				{
					Shape _shape = _polygon[it];
					meshVector[meshID]->BeginPolygon();
					for (int j = 0; j < _shape.size(); ++j)
					{
						meshVector[meshID]->AddPolygon(_shape.getVertexAt(j));
					}
					meshVector[meshID]->EndPolygon();
				}
			}
			meshVector[meshID]->RemovePolygon(i);
		}
	}
}

void splitHiddenFace(std::vector<FbxMesh*>meshVector)
{
	int numPolygon = 0;
	unsigned numMesh = meshVector.size();
	for (unsigned i = 0; i < numMesh; ++i)
	{
		offSet.push_back(numPolygon);
		numPolygon += meshVector[i]->GetPolygonCount();
	}
	std::vector<Shape>shapes(numPolygon);
	std::vector<int>isChanged(numPolygon, -1);//-1表示没改变，1表示该边
	convertMeshPolygonToShape(meshVector, shapes);
	splitShapeInfo(meshVector, shapes, isChanged);
	removeHiddenShape(meshVector, shapes, isChanged, numPolygon);
}
