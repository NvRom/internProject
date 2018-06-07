/*
GJK算法实现
*/
#include "GJK.h"
#include "stdafx.h"
#include <assert.h>
/*------------------------Point相关函数------------------------*/
//Point初始化函数
Point::Point(double _x, double _y) :x(_x), y(_y) {};
//点积操作
double Point::dot(Point _p)
{
	return this->x*_p.x + this->y*_p.y;
}
//相反方向的向量
Point& Point::nagetive()
{
	Point _p(-1 * this->x, -1 * this->y);
	return _p;
}
//重载操作符
Point& Point::operator-(Point& rhs)
{
	Point _point(this->x - rhs.x, this->y - rhs.y);
	return _point;
}
//标量乘积
Point& Point::scalar(double rhs)
{
	Point _point(this->x * rhs, this->y * rhs);
	return _point;
}
//重新设置向量值
void Point::set(Point _p)
{
	this->x = _p.x;
	this->y = _p.y;
}
/*------------------------结束Point相关------------------------*/

/*------------------------Polygon相关函数------------------------*/
//Polygon初始化函数
Polygon::Polygon(vector<Point>polygonData) :_mData(polygonData), _size(polygonData.size()) {};
//得到多边形的中心
Point Polygon::getPolygonCenter()
{
	double _x = 0.0, _y = 0.0;
	for (int i = 0; i < _size; ++i)
	{
		_x += _mData[i].x;
		_y += _mData[i].y;
	}
	return Point(_x / _size, _y / _size);
}
//得到多边形中方向direct最远的点
Point Polygon::getFarthestPointInDirection(Point direct)
{
	Point _p = _mData[0];
	double distance = direct.dot(_mData[0]);
	for (int i = 1; i < _mData.size(); ++i)
	{
		double _distance = direct.dot(_mData[i]);
		if (distance < _distance)
		{
			_p = _mData[i];
			distance = _distance;
		}
	}
	return _p;
}
/*------------------------结束Polygon相关------------------------*/

/*------------------------simplePolygon相关函数------------------------*/
//插入点_p
void simplePolygon::insert(Point _p)
{
	_mData.push_back(_p);
	++_size;
}
//删除点_p
void simplePolygon::remove(Point _p)
{
	for (vector<Point>::iterator iter = _mData.begin(); iter != _mData.end(); ++iter)
	{
		if (iter->x == _p.x && iter->y == _p.y)
		{
			_mData.erase(iter);
			--_size;
			return;
		}
	}
}
//得到最新插入的点
Point simplePolygon::getLast()
{
	return _mData[_size - 1];
};
//得到倒数第二个插入的点坐标
Point simplePolygon::getB()
{
	assert(_size >= 2);
	return _mData[_size - 2];
};
//得到倒数第三个插入的点坐标
Point simplePolygon::getC()
{
	assert(_size >= 3);
	return _mData[_size - 3];
};
//得到simplePolygon大小
int simplePolygon::size()
{
	return _size;
};
/*------------------------结束simplePolygon相关------------------------*/

//2d点：向量三重积 (A*B)*C = B(A·C) - A(B·C)
Point tripleProduct(Point A, Point B, Point C)
{
	Point p = B.scalar(A.dot(C));
	Point q = A.scalar(B.dot(C));
	return p - q;
}

Point support(Polygon& polygon_1, Polygon& polygon_2, Point& direct)
{
	Point p1 = polygon_1.getFarthestPointInDirection(direct);
	Point p2 = polygon_2.getFarthestPointInDirection(direct.nagetive());
	return p1 - p2;
}

bool containsOrigin(simplePolygon& simple, Point& direct)
{
	Point a = simple.getLast();
	Point ao = a.nagetive();
	if (simple.size() == 3)//说明是三角形
	{
		Point b = simple.getB();
		Point c = simple.getC();
		Point ab = b - a;
		Point ac = c - a;
		Point abPrep = tripleProduct(ac, ab, ab);
		Point acPrep = tripleProduct(ab, ac, ac);
		if (abPrep.dot(ao) >= 0)
		{
			simple.remove(c);
			direct.set(abPrep);
		}
		else
		{
			if (acPrep.dot(ao) >= 0)
			{
				simple.remove(b);
				direct.set(acPrep);
			}
			else
			{
				return true;
			}
		}
	}
	else
	{
		Point b = simple.getB();
		Point ab = b - a;
		Point abPrep = tripleProduct(ab, ao, ab);
		direct.set(abPrep);
	}
	return false;
}

Point initDirection(Polygon polygon_1, Polygon polygon_2)
{
	Point s_1 = polygon_1.getPolygonCenter();
	Point s_2 = polygon_2.getPolygonCenter();
	return s_2 - s_1;
}

bool GJK(Polygon polygon_1, Polygon polygon_2)
{
	Point direct = initDirection(polygon_1, polygon_2);
	simplePolygon simple;
	simple.insert(support(polygon_1, polygon_2, direct));
	direct = direct.nagetive();
	while (true)
	{
		simple.insert(support(polygon_1, polygon_2, direct));
		if (simple.getLast().dot(direct) <= 0)
		{
			return false;
		}
		else
		{
			if (containsOrigin(simple, direct))
			{
				return true;
			}
		}
	}
}