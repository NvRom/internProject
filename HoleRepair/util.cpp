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

FbxVector4 normalize(FbxVector4 & A)
{
	double sum = sqrt(A[0] * A[0] + A[1] * A[1] + A[2] * A[2]);
	A[0] /= sum;
	A[1] /= sum;
	A[2] /= sum;
	return A;
}

//计算两个向量的点乘
double dot(const FbxVector4 &vector_1, const FbxVector4 &vector_2)
{
	return vector_1[0] * vector_2[0] + vector_1[1] * vector_2[1] + vector_1[2] * vector_2[2];
}

FbxVector4 cross(const FbxVector4 vector_1, const FbxVector4 vector_2)
{
	FbxVector4 p;
	p[0] = vector_1[1] * vector_2[2] - vector_1[2] * vector_2[1];
	p[1] = vector_1[2] * vector_2[0] - vector_1[0] * vector_2[2];
	p[2] = vector_1[0] * vector_2[1] - vector_1[1] * vector_2[0];
	return p;
}

//判断两个点是否相邻
bool theNearPoint(FbxVector4 p, FbxVector4 q)
{
	return (p[0] - q[0])*(p[0] - q[0]) + (p[1] - q[1])*(p[1] - q[1]) + (p[2] - q[2])*(p[2] - q[2]) < THRESHOLD;
}

//根据面的法向量以及面上的一点，计算面的表达式
Plane computerPlane(FbxVector4 normal, FbxVector4 point)
{
	Plane p;
	p._normal = normalize(normal);
	p._distance = dot(p._normal, point);
	return p;
}

//计算向量的长度
double celVectorLength(FbxVector4 _vector)
{
	return sqrt(_vector[0] * _vector[0] + _vector[1] * _vector[1] + _vector[2] * _vector[2]);
}

//计算两点之间的距离
double celDistanceBetweenTwoPoints(FbxVector4 p1, FbxVector4 p2)
{
	FbxVector4 _p = p1 - p2;
	return sqrt(dot(_p, _p));
}

Ring::Ring() :_size(0) {};
Ring::Ring(std::vector<Edge> _mData) :_mData(_mData), _size(_mData.size()) {}
bool Ring::insert(Edge _edge)
{
	_mData.push_back(_edge);
	++_size;
	return true;
}
;

Edge::Edge(int point1, int point2, int UV1, int UV2, FbxVector4 normal1, FbxVector4 normal2, Plane plane) 
	:controlPoint{ point1,point2 }, UVIndex{ UV1,UV2 }, normal{ normal1,normal2 }, plane(plane){};
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
