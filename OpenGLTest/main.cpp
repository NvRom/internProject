#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include "gjk.h"

int main()
{
	Point v1(0, 0, 0);
	Point v2(0, 2, 0);
	Point v3(2, 2, 0);
	Point v4(2, 0, 0);

	Point p1(0, 0, 1);
	Point p2(0, 2, 1);
	Point p3(2, 2, 1);
	Point p4(2, 0, 1);

	std::vector<Point> a{ v1,v2,v4 };
	std::vector<Point> b{ p1,p2,p4 };
	Shape A(a), B(b);
	bool sdf = gjk(a, b);
	
}