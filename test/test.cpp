// test.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

using namespace std;

int main()
{
	Point p1(0, 0);
	Point p2(0, 1);
	Point p3(1, 1.5);

	Point p4(0, 2);
	Point p5(0, 0);
	Point p6(2, 0);
	Point p7(2, 2);
	vector<Point>s_1{ p1,p2,p3 };
	vector<Point>s_2{p4, p5, p6, p7};
	Polygon polygon_1(s_1);
	Polygon polygon_2(s_2);
	bool isculling = GJK(polygon_1, polygon_2);

    return 0;
}

