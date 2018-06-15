/*
//determines the collision of two convex points in 3d

point point1[] = {p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z}; // side of a tetrahedron defined by three (end) points: P1(2.x, t2.y, t2.z), P2(t1.x, t1.y, t1.z), P3(t3.x, t3.y, t3.z)
point point2[] = {size, size, size,  size, size, -size,  size, -size, -size,  size, -size, size }; // side of a cube defined by four (end) points: P1(size, size, size), P2(size, size, -size), P3(size, -size, -size), P4(size, -size, size)

int dim_ts = sizeof(point1)/sizeof(*point1); // (amount of points P_i of the point 1) -> here: 3
int dim_point2 = sizeof(point2)/sizeof(*point2); // (amount of points P_i of the point 2) -> here: 4

bool check_collision = gjk(point1, point2, dim_ts, dim_point2); // 0 ... no collision, 1 ... collision
*/

#ifndef __GJK_H_INCLUDED__
#define __GJK_H_INCLUDED__
#include <fbxsdk.h>
#include <vector>
const double PI = acos(-1.0f);
const double EXPANSION = 0.01;

// GJK functions


class Point
{
public:
	double x;
	double y;
	double z;

	Point() : x(0.0), y(0.0), z(0.0) {};
	Point(FbxVector4 p) :x(p[0]), y(p[1]), z(p[2]) {};
	Point(double x, double y, double z) :x(x), y(y), z(z) {};
	Point negate();
	double dot(Point d);
	Point cross(Point cpt);
	void set(Point set);
	Point operator-(Point rhs);
};

class Shape
{
public:
	Shape(std::vector<Point> shape, FbxVector4 normal) :shape(shape), _size(shape.size()), normal(normal) {};
	void set(std::vector<Point>shape, FbxVector4 normal) { this->shape = shape; this->normal = normal; };
	Shape expansion();
	Point& operator[](unsigned index);
	unsigned size() { return _size; };
	FbxVector4 getNormal() { return normal; };
private:
	std::vector<Point> shape;
	FbxVector4 normal;
	unsigned _size;
};


// class & functions for the simplex(storing the points)
class simplex
{

	public:
		void add(Point simplex_add); // add a new point to the simplex (list)
		void set_zero(void);         // (re)sets all points in the simplex-list to zero
//		void print(void);            // prints all the points in the simplex
		void del(int id);	           // delete vector from the simplex with given id(id = 0, 1, 2)
		Point getLast(void);         // returns the last added point
		Point get(int id);           // returns the x/y/z - values of a given index of the simples with id (= 0 ... 3)
		int size(void);              // returns the amount of points in the simplex list (0 ... 4)

//		void draw(int id, double size); // draw a small rectangle at the given index of the simplex
//		void draw_lines(int id1, int id2); // draw a connection line between the two simplex entries

	private:
		double x[4];
		double y[4];
		double z[4];
};
// class & functions for the simplex(storing the Points)

void normalize(FbxVector4& normal);
bool gjk(Shape A, Shape B);

#endif
