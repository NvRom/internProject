#pragma once
//将FBXMesh中的polygon转化为Shape，转化后的结果保存在shapes里
void convertMeshPolygonToShape(std::vector<FbxMesh*>meshVector, std::vector<Shape>&shapes);

//判断两个Shape是否相邻
//相邻的依据是:两个Shape拥有共同的点
bool neighborShape(Shape A, Shape B);

//判断点跟多边形的位置关系，分为在多边形上，多边形前面和多边形后面
int classifyPointToPlane(Point p, Shape shape);

//拉格朗日五点积判断点是否在三角形中
bool PointInsideTriangle(Point a, Point b, Point c, Point p);

//判断点是否在多边形内部（包含边上），若在内部，返回true，否则返回false
//此函数会调用PointInsideTriangle函数，将polygon分割成triangle简化计算
bool PointInsidePolygon(Point returnPoint, Shape collisionFace);

//由startPoint和endPoint组成的线段，是否与collisionFace相交。若相交，则将交点保存在p中，并返回true，否则返回false
//此函数会调用PointInsidePolygon函数，同时被_update调用
bool intersectSegmentPolygon(Point startPoint, Point endPoint, Point& p, Shape collisionFace, Point prePoint);

//判断collisionFace是否有点恰好存在detecFace上
//如果有这样的点，则保存在_p中
//在_update函数中，当发现detectFace中的线段没有跟collisionFace相交时，则调用此函数
bool findInnerPointInDetecFace(Shape detectFace, Shape collisionFace, Point& _p, Point prePoint);

//更新原来detectFace信息，成功返回true，失败返回false
//controlPointIndex：lmesh中controlPoint的个数，可以认为是添加新的controlPoint的下标
//startPoint,endPoint：线段的起点和终点，然后与collisionFace进行碰撞检测，若得到“合法”的交点则存于_p中
//_p：若_update函数返回true，则将线段与collisionFace的交点存于_p中
//collisionFace和detectFace：冲突面和检测面
//_shape：若返回true，则将新的shape信息保存在_shape
bool _update(unsigned& controlPointIndex, Point startPoint, Point endPoint, Point _p, Shape collisionFace, Shape detectFace, Shape& _shape);

//与detectFace发生碰撞的面只有一个，检测是否属于“合法”的碰撞。若“合法”，则调用_update函数更新原来面的信息
//detectFace:检测面，如果计算得到与collisionFace有“合法”碰撞点，则将碰撞点更新到detecFace
//collisionFace：冲突面，与检测面一起计算得到碰撞点
bool updateShapeByOneCollisionFace(FbxMesh* lMesh, Shape& detectFace, Shape collisionFace);

//创建八叉树，里面包含数据
Node* buildOcTree(std::vector<FbxMesh*>&meshVector);

//遍历OcTree，进行碰撞检测
//一个对象属于current，则需要跟他进行碰撞检测的对象有：current的祖先结点、current结点的其他部分以及current的孩子结点里的对象
//发生碰撞的对象记录在全局变量collisionTable里
void testAllCollisions(Node *root, std::vector<Shape>&shapes);

//根据对象的碰撞信息，除去对象中不可见的部分，将可见部分的信息保存在全局变量additionPolygon里
void splitShapeInfo(std::vector<FbxMesh*>&meshVector, std::vector<Shape>&shapes, std::vector<int>&isChanged);

//根据additionPolygon里面的信息，更新以及删除不可见面
void removeHiddenShape(std::vector<FbxMesh*>meshVector, std::vector<Shape>shapes, std::vector<int>isChanged, int numPolygon);

//入口函数,只接受一个参数std::vector<FbxMesh*>
void splitHiddenFace(std::vector<FbxMesh*>meshVector);