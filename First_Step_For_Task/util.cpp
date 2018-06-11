#include "stdafx.h"

int min(int p, int q) {
	return p < q ? p : q;
}

int max(int p, int q) {
	return p > q ? p : q;
}

void swap(int & p, int & q)
{
	int temp = p;
	p = q;
	q = temp;
}

double dot(FbxVector4 &normal_1, FbxVector4 &normal_2)
{
	return normal_1[0] * normal_2[0] + normal_1[1] * normal_2[1] + normal_1[2] * normal_2[2];
}

//判断两个点是否相邻
bool theNearPoint(FbxVector4 p, FbxVector4 q)
{
	return (p[0] - q[0])*(p[0] - q[0]) + (p[1] - q[1])*(p[1] - q[1]) + (p[2] - q[2])*(p[2] - q[2]) < THRESHOLD;
}

Plane computerPlane(FbxVector4 normal, FbxVector4 point)
{
	Plane p;
	Point3D _p(normal),_v(point);
	p.n = normalize(_p);
	p.d = dot(p.n, _v);
	return p;
}

Circle::Circle() :_size(0) {};
Circle::Circle(std::vector<Edge> _mData) :_mData(_mData), _size(_mData.size()) {}
bool Circle::insert(Edge _edge)
{
	_mData.push_back(_edge);
	++_size;
	return true;
}
;

Edge::Edge(int point1, int point2, int UV1, int UV2, FbxVector4 normal1, FbxVector4 normal2, Plane plane) :controlPoint{ point1,point2 }, UVIndex{ UV1,UV2 }, normal{ normal1,normal2 }, plane(plane){};
Edge::Edge(int point1, int point2) :controlPoint{ point1,point2 } {};

bool edgeSortCriterion::operator()(const Edge& edge_1, const Edge& edge_2)const
{
	//只比较controlPoint是否相等
	if (edge_1.controlPoint[0] < edge_2.controlPoint[0])
	{
		return true;
	}
	else if (edge_1.controlPoint[0] == edge_2.controlPoint[0])
	{
		if (edge_1.controlPoint[1] < edge_2.controlPoint[1])
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}
