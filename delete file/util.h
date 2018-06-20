#pragma once

//模型中存在远处看起来像一个点，放大后看是两个点的情况。点与点的距离如果小于0.01，则认为属于一个点
//更为一般的是，合理的阈值设定应该不破坏原来的多边形关系
//假设threshold为0.01，表示坐标轴上的差值要小于0.01才可能被认为是相邻点
const double THRESHOLD = 0.0002;
const double HASHVALUESTEPLENGTH = 0.00000005;//hash函数发生冲突时，步长
const double INTERSECT_PLANE_EPSILON = 0.00001;//判断三个面是否相交于一点的阈值
const double ACOS = 0.99984769515;//二面角度数在[179,180]之间时，认为两个平面平行或同一平面；两个法向量夹角为1°时，acos值为0.99984769515；

int min(int p, int q);
int max(int p, int q);
void swap(int& p, int& q);

struct Edge
{
	int controlPoint[2];
	int UVIndex[2];
	FbxVector4 normal[2];
	Plane plane;
	Edge(int point1, int point2, int UV1, int UV2, FbxVector4 normal1, FbxVector4 normal2, Plane plane);
	Edge(int point1, int point2);

};

struct Circle
{
	std::vector<Edge> _mData;
	unsigned _size;
	Circle();
	Circle(std::vector<Edge> _mData);
	bool insert(Edge _edge);
};

class edgeSortCriterion
{
public:
	bool operator()(const Edge& edge_1, const Edge& edge_2)const;
};

double dot(FbxVector4& normal_1, FbxVector4& normal_2);

bool theNearPoint(FbxVector4 p, FbxVector4 q);

//生成Plane
Plane computerPlane(FbxVector4 normal, FbxVector4 point);
//计算向量的长度
double celVectorLength(FbxVector4 _vector);

double celDistanceBetweenTwoPoints(FbxVector4 p1,FbxVector4 p2);