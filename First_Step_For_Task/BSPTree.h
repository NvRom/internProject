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

//3d��
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

//��,<n,d>��ʽ��nΪ��������dΪ��ԭ��ľ���
struct Plane
{
	Point3D n;
	double d;
};

//BSPNode��ṹ
struct BSPNode
{
	BSPNode* _child[2];//child[0]��ʾfront��front[1]��ʾbehind
	Plane _splitPlane;
	std::vector<Polygon3D>_Polygons;
	BSPNode(BSPNode* frontNode, BSPNode* behindNode, Plane splitPlane);
	BSPNode(std::vector<Polygon3D>polygons);
};

//����plane
Plane computerPlane(Point3D a, Point3D b, Point3D c);

//��λ������
Point3D normalize(Point3D& A);

//���
double dot(Point3D normal, Point3D a);

//���
Point3D& cross(Point3D A, Point3D B);

//�߶���ƽ��Ľ��㣨��ȷ���ཻ��
//a,b��ʾ�߶ε��������㣬splitPlane��ʾ�ָ���
Point3D intersectEdgeAgainstPlane(Point3D& a, Point3D& b, Plane& splitPlane);

//���������ڷָ����ཻ�����ݷָ����λ�ã��ָ��ǰ��������,�õ��Ľ��������frontPart��behindPart��
void splitPolygon(Polygon3D& poly, Plane splitPlane, Polygon3D& frontPart, Polygon3D& behindPart);

//�жϵ���ָ���Ĺ�ϵ����Ϊ�����桢�����������
int const classifyPointToPlane(Point3D& p, Plane& plane);

//�ж϶������ָ���Ĺ�ϵ
int classifyPolygonToPlane(Polygon3D &poly, Plane &splitPlane);

//��3dģ�Ͷ�����еõ�����ʽ
Plane getPlaneFromPolygon(Polygon3D polygon);

//ѡ����ѷָ���
Plane pickSplittingPlane(std::vector<Polygon3D>&polygons);

//����BSP��
BSPNode* buildBSPTree(std::vector<Polygon3D>&polygons, int depth = 0);
