#pragma once

const double PI = acos(-1.0f);
const double EXPANSION = 0.01;
const double POINTDISTANCE = 0.01;//小于这个阈值的点视为同一个点
const double PLANETHICNESS = 0.0001;

//基本型，点/向量
class Point
{
public:
	double x;
	double y;
	double z;
	//构造函数
	Point() : x(0.0), y(0.0), z(0.0) {};
	Point(FbxVector4 p) :x(p[0]), y(p[1]), z(p[2]) {};
	Point(double x, double y, double z) :x(x), y(y), z(z) {};
	//向量取反
	Point negate();
	//点乘
	double dot(Point d);
	//叉乘
	Point cross(Point cpt);
	//重设点/向量
	void set(Point p);
	//比较与目标点是否为同一个点，出于健壮性考虑，有一个容错值（EPSILON）
	bool equal(Point& p);
	//操作符-重载
	Point operator-(Point rhs);
	//操作符+重载
	Point operator+(Point rhs);
};

//论文中的shape
class Shape
{
public:
	Shape() :_size(0) {};
	Shape(std::vector<Point> shape, FbxVector4 normal) :_shape(shape), _size(shape.size()), normal(normal) {};
	//"膨胀"操作，沿shape的正方向移动一定距离（EXPANSION）
	Shape expansion();
	//运算符重载
	Point& operator[](unsigned index);
	//返回shape中点的个数
	unsigned size() { return _size; };
	//返回shape的法向量
	FbxVector4 getNormal() { return normal; };
	//重新设置法向量
	void setNormal(FbxVector4 normal) { this->normal = normal; };
	//重置shape
	void set(std::vector<Point>shape, FbxVector4 normal, std::vector<int>vertexIndex, unsigned _size) { this->_shape = shape; this->normal = normal; this->vertexIndex = vertexIndex; this->_size = _size; };
	//添加新的点
	void insert(Point p, int index) { _shape.push_back(p); ++_size; vertexIndex.push_back(index); };
	//返回shape中点在mesh中的索引，其中0<<index<<Shape::size()
	int getVertexAt(unsigned index) { return vertexIndex[index]; };
private:
	std::vector<Point> _shape;
	std::vector<int> vertexIndex;
	FbxVector4 normal;
	unsigned _size;
};

//论文中的simplex型
class simplex
{
public:
	//添加新的点
	void add(Point simplex_add);
	//重新初始化simplex
	void set_zero(void);
	//删除id所标识的点，注意：不能删除最新添加的点，0<=id<=2
	void del(int id);
	//返回最近添加的点
	Point getLast(void);
	//返回id所标识的点，0<=id<=3
	Point get(int id);
	//返回simplex中点的个数
	int size(void);
private:
	double x[4];
	double y[4];
	double z[4];
};

//v是向量，返回向量的绝对值
double absoluteValue(Point v);

//单位向量
void normalize(FbxVector4& normal);

//点乘
double dotProd(double x1, double y1, double z1, double x2, double y2, double z2);

//向量的三重积
Point tripleCrossProd(Point a, Point b, Point c);

//返回shape的中心点，用于gjk算法初始化方向
Point middlePoint(Shape A);

//论文中的函数，返回Shape A中沿方向向量最远的点。该函数被support函数所调用
//param:direction：方向向量
Point getFarthestPointInDirection(Shape A, Point direction);

//论文中的support函数，得到待加入simplex的点
//param:direction：方向向量
Point support(Shape A, Shape B, Point direction);

//gjk算法的核心部分，判断simplex型是否包含原点
//param:simplex:论文中的simplex型，3D条件下，包含四个点
//param:d：方向向量，执行期间会保持更新
bool containsOrigin(simplex& simplex, Point& d);

//gjk算法的入口函数，返回A和B是否发生碰撞
bool gjk(Shape A, Shape B);
