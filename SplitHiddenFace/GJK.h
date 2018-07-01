#pragma once

const double PI = acos(-1.0f);
const double EXPANSION = 0.01;
const double POINTDISTANCE = 0.01;//С�������ֵ�ĵ���Ϊͬһ����
const double PLANETHICNESS = 0.0001;

//�����ͣ���/����
class Point
{
public:
	double x;
	double y;
	double z;
	//���캯��
	Point() : x(0.0), y(0.0), z(0.0) {};
	Point(FbxVector4 p) :x(p[0]), y(p[1]), z(p[2]) {};
	Point(double x, double y, double z) :x(x), y(y), z(z) {};
	//����ȡ��
	Point negate();
	//���
	double dot(Point d);
	//���
	Point cross(Point cpt);
	//�����/����
	void set(Point p);
	//�Ƚ���Ŀ����Ƿ�Ϊͬһ���㣬���ڽ�׳�Կ��ǣ���һ���ݴ�ֵ��EPSILON��
	bool equal(Point& p);
	//������-����
	Point operator-(Point rhs);
	//������+����
	Point operator+(Point rhs);
};

//�����е�shape
class Shape
{
public:
	Shape() :_size(0) {};
	Shape(std::vector<Point> shape, FbxVector4 normal) :_shape(shape), _size(shape.size()), normal(normal) {};
	//"����"��������shape���������ƶ�һ�����루EXPANSION��
	Shape expansion();
	//���������
	Point& operator[](unsigned index);
	//����shape�е�ĸ���
	unsigned size() { return _size; };
	//����shape�ķ�����
	FbxVector4 getNormal() { return normal; };
	//�������÷�����
	void setNormal(FbxVector4 normal) { this->normal = normal; };
	//����shape
	void set(std::vector<Point>shape, FbxVector4 normal, std::vector<int>vertexIndex, unsigned _size) { this->_shape = shape; this->normal = normal; this->vertexIndex = vertexIndex; this->_size = _size; };
	//����µĵ�
	void insert(Point p, int index) { _shape.push_back(p); ++_size; vertexIndex.push_back(index); };
	//����shape�е���mesh�е�����������0<<index<<Shape::size()
	int getVertexAt(unsigned index) { return vertexIndex[index]; };
private:
	std::vector<Point> _shape;
	std::vector<int> vertexIndex;
	FbxVector4 normal;
	unsigned _size;
};

//�����е�simplex��
class simplex
{
public:
	//����µĵ�
	void add(Point simplex_add);
	//���³�ʼ��simplex
	void set_zero(void);
	//ɾ��id����ʶ�ĵ㣬ע�⣺����ɾ��������ӵĵ㣬0<=id<=2
	void del(int id);
	//���������ӵĵ�
	Point getLast(void);
	//����id����ʶ�ĵ㣬0<=id<=3
	Point get(int id);
	//����simplex�е�ĸ���
	int size(void);
private:
	double x[4];
	double y[4];
	double z[4];
};

//v�����������������ľ���ֵ
double absoluteValue(Point v);

//��λ����
void normalize(FbxVector4& normal);

//���
double dotProd(double x1, double y1, double z1, double x2, double y2, double z2);

//���������ػ�
Point tripleCrossProd(Point a, Point b, Point c);

//����shape�����ĵ㣬����gjk�㷨��ʼ������
Point middlePoint(Shape A);

//�����еĺ���������Shape A���ط���������Զ�ĵ㡣�ú�����support����������
//param:direction����������
Point getFarthestPointInDirection(Shape A, Point direction);

//�����е�support�������õ�������simplex�ĵ�
//param:direction����������
Point support(Shape A, Shape B, Point direction);

//gjk�㷨�ĺ��Ĳ��֣��ж�simplex���Ƿ����ԭ��
//param:simplex:�����е�simplex�ͣ�3D�����£������ĸ���
//param:d������������ִ���ڼ�ᱣ�ָ���
bool containsOrigin(simplex& simplex, Point& d);

//gjk�㷨����ں���������A��B�Ƿ�����ײ
bool gjk(Shape A, Shape B);
