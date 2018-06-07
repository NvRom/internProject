#pragma once
#include <vector>
using namespace std;
//点的结构
struct Point
{
	float x;
	float y;
	Point(float _x = -0.1, float _y = -0.1);
	float dot(Point _p);
	Point& nagetive();
	Point& operator-(Point& rhs);
	Point& scalar(float rhs);
	void set(Point _p);
};

//形状
struct Polygon
{
	vector<Point>_mData;
	const int _size;
	Polygon(vector<Point>polygonData);
	Point getPolygonCenter();
	Point getFarthestPointInDirection(Point direct);
};

//书中提到的简单图形
class simplePolygon
{
private:
	vector<Point>_mData;
	int _size = 0;
public:
	void insert(Point _p);
	void remove(Point _p);
	Point getLast();
	Point getB();
	Point getC();
	int size();
};

//向量三重积 (A*B)*C = B(A·C) - A(B·C)
Point tripleProduct(Point A, Point B, Point C);
//书中提到的support函数
Point support(Polygon& polygon_1, Polygon& polygon_2, Point& direct);

//判断是否包含
bool containsOrigin(simplePolygon& simple, Point& direct);

//初始化方向
Point initDirection(Polygon polygon_1, Polygon polygon_2);

//碰撞检测算法
bool GJK(Polygon polygon_1, Polygon polygon_2);