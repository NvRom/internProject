#pragma once

//ģ���д���Զ����������һ���㣬�Ŵ���������������������ľ������С��0.01������Ϊ����һ����
//��Ϊһ����ǣ��������ֵ�趨Ӧ�ò��ƻ�ԭ���Ķ���ι�ϵ
//����thresholdΪ0.01����ʾ�������ϵĲ�ֵҪС��0.01�ſ��ܱ���Ϊ�����ڵ�
const double THRESHOLD = 0.0002;
const double HASHVALUESTEPLENGTH = 0.00000005;//hash����������ͻʱ������
const double INTERSECT_PLANE_EPSILON = 0.00001;//�ж��������Ƿ��ཻ��һ�����ֵ
const double ACOS = 0.99984769515;//����Ƕ�����[179,180]֮��ʱ����Ϊ����ƽ��ƽ�л�ͬһƽ�棻�����������н�Ϊ1��ʱ��acosֵΪ0.99984769515��

int min(int p, int q);
int max(int p, int q);
void swap(int& p, int& q);

struct Plane
{
	FbxVector4 _normal;
	double _distance;
};

struct Edge
{
	int controlPoint[2];
	int UVIndex[2];
	FbxVector4 normal[2];
	Plane plane;
	Edge(int point1, int point2, int UV1, int UV2, FbxVector4 normal1, FbxVector4 normal2, Plane plane);
	Edge(int point1, int point2);

};

struct Ring
{
	std::vector<Edge> _mData;
	unsigned _size;
	Ring();
	Ring(std::vector<Edge> _mData);
	bool insert(Edge _edge);
};


class edgeSortCriterion
{
public:
	bool operator()(const Edge& edge_1, const Edge& edge_2)const;
};

//��λ������
FbxVector4 normalize(FbxVector4& A);

//�������
double dot(const FbxVector4 &vector_1, const FbxVector4 &vector_2);

FbxVector4 cross(const FbxVector4 vector_1, const FbxVector4 vector_2);

//�ж��Ƿ����ٽ���
bool theNearPoint(FbxVector4 p, FbxVector4 q);

//������ķ������Լ����ϵ�һ�㣬������ı��ʽ
Plane computerPlane(FbxVector4 normal, FbxVector4 point);

//���������ĳ���
double celVectorLength(FbxVector4 _vector);

//��������֮��ľ���
double celDistanceBetweenTwoPoints(FbxVector4 p1, FbxVector4 p2);
