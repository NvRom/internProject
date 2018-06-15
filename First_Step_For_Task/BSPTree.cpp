/*
BSPTreeʵ��
*/
#include "stdafx.h"
#include <assert.h>
/*------------------------BSPNode��غ���------------------------*/
//BSPNode���캯��
BSPNode::BSPNode(BSPNode* frontNode, BSPNode* behindNode, Plane splitPlane)
	:_child{ frontNode, behindNode }, _splitPlane(splitPlane){};

BSPNode::BSPNode(vector<Polygon3D>polygons)
	:_Polygons(polygons), _child{ nullptr,nullptr } {};
/*------------------------����BSPNode���------------------------*/

/*------------------------Point3D��غ���------------------------*/
//3d�㹹�캯��
Point3D::Point3D() {};

Point3D::Point3D(double _x, double _y, double _z) :x(_x), y(_y), z(_z) {};

Point3D::Point3D(FbxVector4 vertex) :x(vertex[0]), y(vertex[1]), z(vertex[2]) {};

//���ز�����-
Point3D& Point3D::operator-(Point3D& rhs)
{
	Point3D _point(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
	return _point;
}

//���ز�����+
Point3D & Point3D::operator+(Point3D& rhs)
{
	Point3D _point(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
	return _point;
}

//�����˻�
Point3D & Point3D::scalar(double rhs)
{
	this->x *= rhs;
	this->y *= rhs;
	this->z *= rhs;
	return *this;
}
////���ز�����=
//Point3D& Point3D::operator=(Point3D& rhs)
//{
//	this->x = rhs.x;
//	this->y = rhs.y;
//	this->z = rhs.z;
//	return *this;
//}
/*------------------------����Point3D���------------------------*/

/*------------------------Polygon3D��غ���------------------------*/
//Polygon3D��ʼ������
Polygon3D::Polygon3D() :_size(0) {};

Polygon3D::Polygon3D(std::vector<Point3D>polygonData) :_mData(polygonData), _size(polygonData.size()) {};

Polygon3D::Polygon3D(std::vector<Point3D> polygonData, int _size) : _size(_size), _mData(polygonData.begin(), polygonData.begin() + _size) {};

//���ز�����[]
Point3D  Polygon3D::operator[](int index)
{
	assert(index < _size && index >= 0);
	return _mData[index];
}

//�����µĽ��
bool Polygon3D::insertPoint(Point3D p)
{
	_mData.push_back(p);
	++_size;
	return true;
}
;
/*------------------------����Polygon3D���------------------------*/

//��λ������
Point3D normalize(Point3D &A)
{
	double sum = sqrt(A.x*A.x + A.y*A.y + A.z*A.z);
	A.x /= sum;
	A.y /= sum;
	A.z /= sum;
	return A;
}

//����Plane
Plane computerPlane(Point3D a, Point3D b, Point3D c)
{
	Plane p;
	p.n = normalize(cross(b - a, c - a));
	p.d = dot(p.n, a);
	return p;
}

//3d�㣺���
Point3D& cross(Point3D A, Point3D B)
{
	Point3D p;
	p.x = A.y*B.z - A.z*B.y;
	p.y = A.z*B.x - A.x*B.z;
	p.z = A.x*B.y - A.y*B.x;
	return p;
}
//3d�㣺���
double dot(Point3D n, Point3D a)
{
	return n.x*a.x + n.y*a.y + n.z*a.z;
}

//�߶���ƽ��Ľ��㣨��ȷ���ཻ��
Point3D intersectEdgeAgainstPlane(Point3D& a, Point3D& b, Plane& splitPlane)
{
	Point3D ab = b - a;
	double t = (splitPlane.d - dot(splitPlane.n, a)) / dot(splitPlane.n, ab);
	assert(t >= 0.0f&&t <= 1.0f);
	Point3D p = ab.scalar(t) + a;
	return p;
}

//����BSP��
BSPNode* buildBSPTree(std::vector<Polygon3D>&polygons, int depth)
{
	//���polygonsΪ�գ�����nullptr
	if (polygons.empty())
		return nullptr;
	int numPolygons = polygons.size();
	//���depth���������ȣ�numPolygonsС����СҶ������ֱ�ӷ���BSPNode
	if (depth >= MAX_DEPTH || numPolygons <= MIN_LEAF_SIZE)
	{
		return new BSPNode(polygons);
	}
	//�õ���ѷָ���
	Plane splitPlane = pickSplittingPlane(polygons);
	vector<Polygon3D>frontPolygons, behindPolygons;
	//��������polygons�����ݷָ��潫polygons�ָ��������
	for (int i = 0; i < numPolygons; ++i)
	{
		Polygon3D poly = polygons[i];
		switch (classifyPolygonToPlane(poly,splitPlane))
		{
		case POLYGON_COPLANAR_WITH_PLANE:
			//������棬������,�����classifyPolygonToPlane�����оͽ����ж�
			break;
		case POLYGON_IN_FRONT_OF_PLANE:
			frontPolygons.push_back(poly);//push_back����������
			break;
		case POLYGON_BEHIND_PLANE:
			behindPolygons.push_back(poly);
			break;
		case POLYGON_STRADDLING_PLANE:
			Polygon3D frontPart, behindPart;
			splitPolygon(poly, splitPlane, frontPart, behindPart);
			frontPolygons.push_back(frontPart);
			behindPolygons.push_back(behindPart);
			break;
		}
	}
	//�ݹ�ִ�й���������������󷵻�root���
	BSPNode* frontTree = buildBSPTree(frontPolygons, depth + 1);
	BSPNode* behindTree = buildBSPTree(behindPolygons, depth + 1);
	return new BSPNode(frontTree, behindTree, splitPlane);
}

Plane getPlaneFromPolygon(Polygon3D polygon)
{
	assert(polygon._size >= 3);
	Plane p = computerPlane(polygon[0], polygon[1], polygon[2]);
	return p;
}

/*
ѡ����ѷָ���
����һ���ָ����Ƿ�Ϊ��ѵķ����кܶ࣬ͨ��������������㣵ĵ㣺
1���������ٵĲ����µ��棬���ָ���Ҫ�������ٵĴ�Խ�����棬�����ζ�������������νṹ�ǳ���
2��������Ҫ�߱��Ϻõ�ƽ���ԣ������ĸ߶ȴ����log(n)�������ζ�ſ���Ҫ��Խ����Ѵ��ڵ���
������õ��Ƕ������߲��ü�Ȩƽ��������������K�������ٻ��ֲ��Ժ�ƽ�⻮�ֲ��Եı���
*/
Plane pickSplittingPlane(std::vector<Polygon3D>&polygons)
{
	const double K = 0.8;
	Plane bestPlane;
	double bestScore = DBL_MAX;
	//�������еĶ���Σ�Ѱ����ѷָ���
	int numPolygon = polygons.size();
	for (int i = 0; i < numPolygon; ++i)
	{
		int numInFront = 0, numBehind = 0, numStraddling = 0;
		Plane plane = getPlaneFromPolygon(polygons[i]);
		for (int j = 0 ; j < numPolygon ; ++j)
		{
			if (i == j) continue;
			switch (classifyPolygonToPlane(polygons[j],plane))
			{
			case POLYGON_COPLANAR_WITH_PLANE:
				//��Ҫ������һ���������в���
				break;
			case POLYGON_IN_FRONT_OF_PLANE:
				++numInFront;
				break;
			case POLYGON_BEHIND_PLANE:
				++numBehind;
				break;
			case POLYGON_STRADDLING_PLANE:
				++numStraddling;
				break;
			}
		}
		//����score���Ƚ��Ƿ�Ϊ��ѷָ���
		double score = K * numStraddling + (1.0f - K)*abs(numInFront - numBehind);
		if (score < bestScore)
		{
			bestScore = score;
			bestPlane = plane;
		}
	}
	return bestPlane;
}


//�жϵ���ָ���Ĺ�ϵ����Ϊ�����桢�����������
//���õķ����ǣ�dot(n,p)�õ���ԭ��ľ���d1�����浽ԭ��ľ������Ƚϣ��ó��Ƿ�������
int const classifyPointToPlane(Point3D& p, Plane& splitPlane)
{
	//��Ϊ���������Ĺ�ϵ�����еĵ㲢�������������ϵ���ȣ�������epsilon������ֵ���ж�
	double dist = dot(p, splitPlane.n) - splitPlane.d;
	if (dist > PLANE_THICKNESS_EPSILON)
	{
		return POINT_IN_FRONT_OF_PLANE;
	}
	if (dist < -PLANE_THICKNESS_EPSILON)
	{
		return POINT_BEHIND_PLANE;
	}
	return POINT_ON_PLANE;
}

//�ж϶����polygon����plane�Ĺ�ϵ
int classifyPolygonToPlane(Polygon3D& poly, Plane& splitPlane)
{
	int numInFront = 0, numBehind = 0;
	int numVertex = poly._size;
	for (int i = 0; i < numVertex; ++i)
	{
		Point3D p = poly[i];
		switch (classifyPointToPlane(p, splitPlane))
		{
		case POINT_IN_FRONT_OF_PLANE:
			++numInFront;
			break;
		case POINT_BEHIND_PLANE:
			++numBehind;
			break;
		default:
			break;
		}
	}
	if (numBehind != 0 && numInFront != 0)
	{
		return POLYGON_STRADDLING_PLANE;
	}
	if (numInFront != 0)
	{
		return POLYGON_IN_FRONT_OF_PLANE;
	}
	if (numBehind != 0)
	{
		return POLYGON_BEHIND_PLANE;
	}
	//˵�����ڻ���ƽ���ϣ����ݶ����polygon�ĳ��򻮷ֵ�ǰ���Ƿ�Ϊ������
	if (dot(getPlaneFromPolygon(poly).n, splitPlane.n) > 0)
	{
		return POLYGON_IN_FRONT_OF_PLANE;
	}
	else
	{
		return POLYGON_BEHIND_PLANE;
	}
}

/*
��֪�����polygon��splitPlane���֣��õ�������
����intersectEdgeAgainstPlane��ʾ�߶�������ཻ��
��Ϊ��������ʾ���ȵĹ�ϵ����Ҫ��������splitPlane��Ϊ����һ����ȵ���
|-------------------------------------------------------|
|A(��һ������)    B(��ǰ����)     �������    �������  |
|-------------------------------------------------------|
|  λ������		  λ������          B           ��      |
|-------------------------------------------------------|
|  λ������		  λ������          B           ��      |
|-------------------------------------------------------|
|  λ�ڱ���		  λ������         J��B         J       |
|-------------------------------------------------------|
|  λ������		  λ������          B           ��      |
|-------------------------------------------------------|
|  λ������		  λ������          B           ��      |
|-------------------------------------------------------|
|  λ�ڱ���		  λ������          B           B       |
|-------------------------------------------------------|
|  λ������		  λ�ڱ���          J          J��B     |
|-------------------------------------------------------|
|  λ������		  λ�ڱ���          ��         A��B     |
|-------------------------------------------------------|
|  λ�ڱ���		  λ�ڱ���          ��          B       |
|-------------------------------------------------------|
*/
void splitPolygon(Polygon3D& poly, Plane splitPlane, Polygon3D& frontPart, Polygon3D& behindPart)
{
	int numInFront = 0, numBehind = 0;
	const int numVertex = poly._size;
	std::vector<Point3D>frontVertex(numVertex * 2);//����ǰ�沿�ֵĽ��
	std::vector<Point3D>behindVertex(numVertex * 2);//������沿�ֵĽ��
	Point3D a = poly[numVertex - 1];
	int aSide = classifyPointToPlane(a, splitPlane);
	for (int i = 0 ; i < numVertex ; ++i)
	{
		Point3D b = poly[i];
		int bSide = classifyPointToPlane(b, splitPlane);
		if (bSide == POINT_IN_FRONT_OF_PLANE)
		{
			if (aSide == POINT_BEHIND_PLANE)
			{
				Point3D j = intersectEdgeAgainstPlane(a, b, splitPlane);
				assert(classifyPointToPlane(j, splitPlane) == POINT_ON_PLANE);
				frontVertex[numInFront++] = behindVertex[numBehind++] = j;
			}
			frontVertex[numInFront++] = b;
		}
		else if (bSide == POINT_BEHIND_PLANE)
		{
			if (aSide == POINT_IN_FRONT_OF_PLANE)
			{
				Point3D j = intersectEdgeAgainstPlane(a, b, splitPlane);
				assert(classifyPointToPlane(j, splitPlane) == POINT_ON_PLANE);
				frontVertex[numInFront++] = behindVertex[numBehind++] = j;
			}else if (aSide == POINT_ON_PLANE)
			{
				behindVertex[numBehind++] = a;
			}
			behindVertex[numBehind++] = b;
		}
		else//b������
		{
			if (aSide == POINT_BEHIND_PLANE)
			{
				behindVertex[numBehind++] = b;
			}
			frontVertex[numInFront++] = b;
		}
		a = b;
		aSide = bSide;
	}
	Polygon3D _FrontPart(frontVertex, numInFront);
	frontPart = _FrontPart;
	Polygon3D _BehindPart(behindVertex, numBehind);
	behindPart = _BehindPart;
}
