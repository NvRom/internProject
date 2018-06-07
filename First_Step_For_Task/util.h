#pragma once

//ģ���д���Զ����������һ���㣬�Ŵ���������������������ľ������С��0.01������Ϊ����һ����
//��Ϊһ����ǣ��������ֵ�趨Ӧ�ò��ƻ�ԭ���Ķ���ι�ϵ
//����thresholdΪ0.01����ʾ�������ϵĲ�ֵҪС��0.01�ſ��ܱ���Ϊ�����ڵ�
const double THRESHOLD = 0.0002;
const double HASHVALUESTEPLENGTH = 0.00000005;

int min(int p, int q);
int max(int p, int q);
void swap(int& p, int& q);

struct Edge
{
	int controlPoint[2];
	int UVIndex[2];
	FbxVector4 normal[2];
	int polygonID;
	Edge(int point1, int point2, int UV1, int UV2, FbxVector4 normal1, FbxVector4 normal2, int polygonID);
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

double dot(FbxVector4 normal_1, FbxVector4 normal_2);
double vectorLength(FbxVector4 normal);