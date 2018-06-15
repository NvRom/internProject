#pragma once
#include <vector>
#include <fbxsdk.h>
enum PolygonToPlane
{
	POLYGON_COPLANAR_WITH_PLANE = 1,
	POLYGON_IN_FRONT_OF_PLANE = 2,
	POLYGON_BEHIND_PLANE = 3,
	POLYGON_STRADDLING_PLANE = 4
};
enum PointTOPlane
{
	POINT_IN_FRONT_OF_PLANE = 1,
	POINT_BEHIND_PLANE = 2,
	POINT_ON_PLANE = 3
};

const int MAX_DEPTH = 10;
const int MIN_LEAF_SIZE = 5;
const double PLANE_THICKNESS_EPSILON = 0.00001;
const int MAX_POINTS = 50;

//3d点
struct Point3D
{
	double x;
	double y;
	double z;
	Point3D();
	Point3D(double _x, double _y, double _z);
	Point3D(FbxVector4 vertex);
	Point3D& operator-(Point3D& rhs);
	Point3D& operator+(Point3D& rhs);
	Point3D& scalar(double rhs);
};

struct Polygon3D
{
	int _size;
	std::vector<Point3D>_mData;
	Polygon3D();
	Polygon3D(std::vector<Point3D>polygonData);
	Polygon3D(std::vector<Point3D>polygonData, int _size);
	Point3D operator[](int index);
	bool insertPoint(Point3D p);
};

//面,<n,d>形式，n为法向量，d为到原点的距离
struct Plane
{
	Point3D n;
	double d;
};

//BSPNode点结构
struct BSPNode
{
	BSPNode* _child[2];//child[0]表示front，front[1]表示behind
	Plane _splitPlane;
	std::vector<Polygon3D>_Polygons;
	BSPNode(BSPNode* frontNode, BSPNode* behindNode, Plane splitPlane);
	BSPNode(std::vector<Polygon3D>polygons);
};

//生成plane
Plane computerPlane(Point3D a, Point3D b, Point3D c);

//单位化向量
Point3D normalize(Point3D& A);

//点乘
double dot(Point3D normal, Point3D a);

//叉乘
Point3D& cross(Point3D A, Point3D B);

//线段与平面的交点（已确认相交）
//a,b表示线段的两个顶点，splitPlane表示分割面
Point3D intersectEdgeAgainstPlane(Point3D& a, Point3D& b, Plane& splitPlane);

//如果多边形于分割面相交，根据分割面的位置，分割出前后两部分,得到的结果保存在frontPart和behindPart中
void splitPolygon(Polygon3D& poly, Plane splitPlane, Polygon3D& frontPart, Polygon3D& behindPart);

//判断点与分割面的关系，分为在正面、背面和在面上
int const classifyPointToPlane(Point3D& p, Plane& plane);

//判断多边形与分割面的关系
int classifyPolygonToPlane(Polygon3D &poly, Plane &splitPlane);

//从3d模型多边形中得到面表达式
Plane getPlaneFromPolygon(Polygon3D polygon);

//选择最佳分割面
Plane pickSplittingPlane(std::vector<Polygon3D>&polygons);

//构造BSP树
BSPNode* buildBSPTree(std::vector<Polygon3D>&polygons, int depth = 0);
