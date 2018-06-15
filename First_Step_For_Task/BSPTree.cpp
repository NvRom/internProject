/*
BSPTree实现
*/
#include "stdafx.h"
#include <assert.h>
/*------------------------BSPNode相关函数------------------------*/
//BSPNode构造函数
BSPNode::BSPNode(BSPNode* frontNode, BSPNode* behindNode, Plane splitPlane)
	:_child{ frontNode, behindNode }, _splitPlane(splitPlane){};

BSPNode::BSPNode(vector<Polygon3D>polygons)
	:_Polygons(polygons), _child{ nullptr,nullptr } {};
/*------------------------结束BSPNode相关------------------------*/

/*------------------------Point3D相关函数------------------------*/
//3d点构造函数
Point3D::Point3D() {};

Point3D::Point3D(double _x, double _y, double _z) :x(_x), y(_y), z(_z) {};

Point3D::Point3D(FbxVector4 vertex) :x(vertex[0]), y(vertex[1]), z(vertex[2]) {};

//重载操作符-
Point3D& Point3D::operator-(Point3D& rhs)
{
	Point3D _point(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
	return _point;
}

//重载操作符+
Point3D & Point3D::operator+(Point3D& rhs)
{
	Point3D _point(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
	return _point;
}

//标量乘积
Point3D & Point3D::scalar(double rhs)
{
	this->x *= rhs;
	this->y *= rhs;
	this->z *= rhs;
	return *this;
}
////重载操作符=
//Point3D& Point3D::operator=(Point3D& rhs)
//{
//	this->x = rhs.x;
//	this->y = rhs.y;
//	this->z = rhs.z;
//	return *this;
//}
/*------------------------结束Point3D相关------------------------*/

/*------------------------Polygon3D相关函数------------------------*/
//Polygon3D初始化函数
Polygon3D::Polygon3D() :_size(0) {};

Polygon3D::Polygon3D(std::vector<Point3D>polygonData) :_mData(polygonData), _size(polygonData.size()) {};

Polygon3D::Polygon3D(std::vector<Point3D> polygonData, int _size) : _size(_size), _mData(polygonData.begin(), polygonData.begin() + _size) {};

//重载操作符[]
Point3D  Polygon3D::operator[](int index)
{
	assert(index < _size && index >= 0);
	return _mData[index];
}

//插入新的结点
bool Polygon3D::insertPoint(Point3D p)
{
	_mData.push_back(p);
	++_size;
	return true;
}
;
/*------------------------结束Polygon3D相关------------------------*/

//单位化向量
Point3D normalize(Point3D &A)
{
	double sum = sqrt(A.x*A.x + A.y*A.y + A.z*A.z);
	A.x /= sum;
	A.y /= sum;
	A.z /= sum;
	return A;
}

//生成Plane
Plane computerPlane(Point3D a, Point3D b, Point3D c)
{
	Plane p;
	p.n = normalize(cross(b - a, c - a));
	p.d = dot(p.n, a);
	return p;
}

//3d点：叉乘
Point3D& cross(Point3D A, Point3D B)
{
	Point3D p;
	p.x = A.y*B.z - A.z*B.y;
	p.y = A.z*B.x - A.x*B.z;
	p.z = A.x*B.y - A.y*B.x;
	return p;
}
//3d点：点乘
double dot(Point3D n, Point3D a)
{
	return n.x*a.x + n.y*a.y + n.z*a.z;
}

//线段与平面的交点（已确认相交）
Point3D intersectEdgeAgainstPlane(Point3D& a, Point3D& b, Plane& splitPlane)
{
	Point3D ab = b - a;
	double t = (splitPlane.d - dot(splitPlane.n, a)) / dot(splitPlane.n, ab);
	assert(t >= 0.0f&&t <= 1.0f);
	Point3D p = ab.scalar(t) + a;
	return p;
}

//构造BSP树
BSPNode* buildBSPTree(std::vector<Polygon3D>&polygons, int depth)
{
	//如果polygons为空，返回nullptr
	if (polygons.empty())
		return nullptr;
	int numPolygons = polygons.size();
	//如果depth大于最大深度，numPolygons小于最小叶子数，直接返回BSPNode
	if (depth >= MAX_DEPTH || numPolygons <= MIN_LEAF_SIZE)
	{
		return new BSPNode(polygons);
	}
	//得到最佳分割面
	Plane splitPlane = pickSplittingPlane(polygons);
	vector<Polygon3D>frontPolygons, behindPolygons;
	//遍历整个polygons，根据分割面将polygons分割成两部分
	for (int i = 0; i < numPolygons; ++i)
	{
		Polygon3D poly = polygons[i];
		switch (classifyPolygonToPlane(poly,splitPlane))
		{
		case POLYGON_COPLANAR_WITH_PLANE:
			//如果共面，看朝向,最好在classifyPolygonToPlane函数中就进行判断
			break;
		case POLYGON_IN_FRONT_OF_PLANE:
			frontPolygons.push_back(poly);//push_back操作！！！
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
	//递归执行构造子树操作，最后返回root结点
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
选择最佳分割面
评价一个分隔面是否为最佳的方法有很多，通常面临着两点相悖的点：
1：尽可能少的产生新的面，即分隔面要尽可能少的穿越其他面，这就意味着生成树的树形结构非常差
2：生成树要具备较好的平衡性，即树的高度大概在log(n)，这就意味着可能要穿越多个已存在的面
这里采用的是对这两者采用加权平均的做法，设立K控制最少划分策略和平衡划分策略的比例
*/
Plane pickSplittingPlane(std::vector<Polygon3D>&polygons)
{
	const double K = 0.8;
	Plane bestPlane;
	double bestScore = DBL_MAX;
	//遍历所有的多边形，寻找最佳分隔面
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
				//需要根据下一步函数进行操作
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
		//计算score，比较是否为最佳分割面
		double score = K * numStraddling + (1.0f - K)*abs(numInFront - numBehind);
		if (score < bestScore)
		{
			bestScore = score;
			bestPlane = plane;
		}
	}
	return bestPlane;
}


//判断点与分割面的关系，分为在正面、背面和在面上
//采用的方法是，dot(n,p)得到到原点的距离d1，与面到原点的距离作比较，得出是否在面上
int const classifyPointToPlane(Point3D& p, Plane& splitPlane)
{
	//因为浮点数误差的关系，所有的点并不是真正意义上的相等，故引入epsilon这个误差值来判断
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

//判断多边形polygon与面plane的关系
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
	//说明面在划分平面上，根据多边形polygon的朝向划分当前面是否为正背面
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
已知多边形polygon被splitPlane划分，得到两个面
函数intersectEdgeAgainstPlane表示线段与面的相交点
因为浮点数表示精度的关系，需要将划分面splitPlane视为具有一定厚度的面
|-------------------------------------------------------|
|A(上一个顶点)    B(当前顶点)     正面输出    背面输出  |
|-------------------------------------------------------|
|  位于正面		  位于正面          B           无      |
|-------------------------------------------------------|
|  位于面上		  位于正面          B           无      |
|-------------------------------------------------------|
|  位于背面		  位于正面         J、B         J       |
|-------------------------------------------------------|
|  位于正面		  位于面上          B           无      |
|-------------------------------------------------------|
|  位于面上		  位于面上          B           无      |
|-------------------------------------------------------|
|  位于背面		  位于面上          B           B       |
|-------------------------------------------------------|
|  位于正面		  位于背面          J          J、B     |
|-------------------------------------------------------|
|  位于面上		  位于背面          无         A、B     |
|-------------------------------------------------------|
|  位于背面		  位于背面          无          B       |
|-------------------------------------------------------|
*/
void splitPolygon(Polygon3D& poly, Plane splitPlane, Polygon3D& frontPart, Polygon3D& behindPart)
{
	int numInFront = 0, numBehind = 0;
	const int numVertex = poly._size;
	std::vector<Point3D>frontVertex(numVertex * 2);//保存前面部分的结点
	std::vector<Point3D>behindVertex(numVertex * 2);//保存后面部分的结点
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
		else//b在面上
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
