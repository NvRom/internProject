#pragma once
#include <vector>
using namespace std;
//��Ľṹ
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

//��״
struct Polygon
{
	vector<Point>_mData;
	const int _size;
	Polygon(vector<Point>polygonData);
	Point getPolygonCenter();
	Point getFarthestPointInDirection(Point direct);
};

//�����ᵽ�ļ�ͼ��
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

//�������ػ� (A*B)*C = B(A��C) - A(B��C)
Point tripleProduct(Point A, Point B, Point C);
//�����ᵽ��support����
Point support(Polygon& polygon_1, Polygon& polygon_2, Point& direct);

//�ж��Ƿ����
bool containsOrigin(simplePolygon& simple, Point& direct);

//��ʼ������
Point initDirection(Polygon polygon_1, Polygon polygon_2);

//��ײ����㷨
bool GJK(Polygon polygon_1, Polygon polygon_2);