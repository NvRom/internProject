#pragma once
//��FBXMesh�е�polygonת��ΪShape��ת����Ľ��������shapes��
void convertMeshPolygonToShape(std::vector<FbxMesh*>meshVector, std::vector<Shape>&shapes);

//�ж�����Shape�Ƿ�����
//���ڵ�������:����Shapeӵ�й�ͬ�ĵ�
bool neighborShape(Shape A, Shape B);

//�жϵ������ε�λ�ù�ϵ����Ϊ�ڶ�����ϣ������ǰ��Ͷ���κ���
int classifyPointToPlane(Point p, Shape shape);

//�������������жϵ��Ƿ�����������
bool PointInsideTriangle(Point a, Point b, Point c, Point p);

//�жϵ��Ƿ��ڶ�����ڲ����������ϣ��������ڲ�������true�����򷵻�false
//�˺��������PointInsideTriangle��������polygon�ָ��triangle�򻯼���
bool PointInsidePolygon(Point returnPoint, Shape collisionFace);

//��startPoint��endPoint��ɵ��߶Σ��Ƿ���collisionFace�ཻ�����ཻ���򽫽��㱣����p�У�������true�����򷵻�false
//�˺��������PointInsidePolygon������ͬʱ��_update����
bool intersectSegmentPolygon(Point startPoint, Point endPoint, Point& p, Shape collisionFace, Point prePoint);

//�ж�collisionFace�Ƿ��е�ǡ�ô���detecFace��
//����������ĵ㣬�򱣴���_p��
//��_update�����У�������detectFace�е��߶�û�и�collisionFace�ཻʱ������ô˺���
bool findInnerPointInDetecFace(Shape detectFace, Shape collisionFace, Point& _p, Point prePoint);

//����ԭ��detectFace��Ϣ���ɹ�����true��ʧ�ܷ���false
//controlPointIndex��lmesh��controlPoint�ĸ�����������Ϊ������µ�controlPoint���±�
//startPoint,endPoint���߶ε������յ㣬Ȼ����collisionFace������ײ��⣬���õ����Ϸ����Ľ��������_p��
//_p����_update��������true�����߶���collisionFace�Ľ������_p��
//collisionFace��detectFace����ͻ��ͼ����
//_shape��������true�����µ�shape��Ϣ������_shape
bool _update(unsigned& controlPointIndex, Point startPoint, Point endPoint, Point _p, Shape collisionFace, Shape detectFace, Shape& _shape);

//��detectFace������ײ����ֻ��һ��������Ƿ����ڡ��Ϸ�������ײ�������Ϸ����������_update��������ԭ�������Ϣ
//detectFace:����棬�������õ���collisionFace�С��Ϸ�����ײ�㣬����ײ����µ�detecFace
//collisionFace����ͻ�棬������һ�����õ���ײ��
bool updateShapeByOneCollisionFace(FbxMesh* lMesh, Shape& detectFace, Shape collisionFace);

//�����˲����������������
Node* buildOcTree(std::vector<FbxMesh*>&meshVector);

//����OcTree��������ײ���
//һ����������current������Ҫ����������ײ���Ķ����У�current�����Ƚ�㡢current�������������Լ�current�ĺ��ӽ����Ķ���
//������ײ�Ķ����¼��ȫ�ֱ���collisionTable��
void testAllCollisions(Node *root, std::vector<Shape>&shapes);

//���ݶ������ײ��Ϣ����ȥ�����в��ɼ��Ĳ��֣����ɼ����ֵ���Ϣ������ȫ�ֱ���additionPolygon��
void splitShapeInfo(std::vector<FbxMesh*>&meshVector, std::vector<Shape>&shapes, std::vector<int>&isChanged);

//����additionPolygon�������Ϣ�������Լ�ɾ�����ɼ���
void removeHiddenShape(std::vector<FbxMesh*>meshVector, std::vector<Shape>shapes, std::vector<int>isChanged, int numPolygon);

//��ں���,ֻ����һ������std::vector<FbxMesh*>
void splitHiddenFace(std::vector<FbxMesh*>meshVector);