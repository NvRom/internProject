// First_Step_For_Task.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <fbxsdk.h>
#include <queue>
#include <map>
#include <assert.h>

enum PointToPolygon
{
	POINT_INFRONT_POLYGON = 1,
	POINT_BEHIND_POLYGON = 2,
	POINT_ON_POLYGON = 3
};

std::vector<unsigned>offSet;
std::vector<Point>prePoints;
std::map<int, std::vector<Shape>>additionPolygon;

//消除隐藏面
void removeHiddenFace(std::vector<FbxMesh*>meshVector)
{
	
}

void convertMeshPolygonToShape(std::vector<FbxMesh*>meshVector, std::vector<Shape>&shapes)
{
	unsigned numMesh = meshVector.size();
	for (unsigned i = 0; i < numMesh; ++i)
	{
		unsigned off_set = offSet[i];
		FbxVector4* controlPointsVertex = meshVector[i]->GetControlPoints();
		int polygonCountInMesh_I = meshVector[i]->GetPolygonCount();
		for (unsigned j = 0;j<polygonCountInMesh_I;++j)
		{
			FBXSDK_printf("第%d结点的坐标为（%f,%f,%f）\n", j, controlPointsVertex[j][0], controlPointsVertex[j][1], controlPointsVertex[j][2]);
		}
		for (int pPolygonIndex = 0; pPolygonIndex < polygonCountInMesh_I; ++pPolygonIndex)
		{
			int polygonVertexCount = meshVector[i]->GetPolygonSize(pPolygonIndex);
			std::vector<Point> shape(polygonVertexCount);
			std::vector<int> vertexIndex(polygonVertexCount);
			for (int pPositionInPolygon = 0; pPositionInPolygon < polygonVertexCount; ++pPositionInPolygon)
			{
				int _vi = meshVector[i]->GetPolygonVertex(pPolygonIndex, pPositionInPolygon);
				Point p(controlPointsVertex[_vi]);
				shape[pPositionInPolygon] = p;
				vertexIndex[pPositionInPolygon] = _vi;
			}
			FbxVector4 normal;
			if (!meshVector[i]->GetPolygonVertexNormal(pPolygonIndex, 0, normal))
			{
				Point ab = shape[1] - shape[0];
				Point ac = shape[2] - shape[0];
				Point n = ab.cross(ac);
				normal[0] = n.x;
				normal[1] = n.y;
				normal[2] = n.z;
			}
			normalize(normal);
			shapes[off_set + pPolygonIndex].set(shape, normal, vertexIndex, shape.size());
		}
	}
}


//判断两个面是否相邻
bool neighborShape(Shape A, Shape B)
{
	for (unsigned i = 0; i < A.size(); ++i)
	{
		for (unsigned j = 0; j < B.size(); ++j)
		{
			if (A.getVertexAt(i)==B.getVertexAt(j))
			{
				return true;
			}
		}
	}
	return false;
}

//判断点跟多边形的关系，分为在多边形上，多边形前面和多边形后面
int classifyPointToPlane(Point p,Shape shape)//返回值应该为在线上、多边形内部、多边形外部
{
	Point _normal = shape.getNormal();
	double dist = p.dot(_normal) - shape[0].dot(_normal);
	if (dist > PLANETHICNESS)
	{
		return POINT_INFRONT_POLYGON;
	}
	else if (dist < -PLANETHICNESS)
	{
		return POINT_BEHIND_POLYGON;
	}
	else
	{
		return POINT_ON_POLYGON;
	}
}

//拉格朗日五点积判断点是否在三角形中
bool PointInsideTriangle(Point a, Point b, Point c, Point p)
{
	a = a - p;
	b = b - p;
	c = c - p;
	if (absoluteValue(a) < POINTDISTANCE || absoluteValue(b) < POINTDISTANCE || absoluteValue(c) < POINTDISTANCE)
	{
		return true;
	}
	double ab = a.dot(b);
	double ac = a.dot(c);
	double bc = b.dot(c);
	double cc = c.dot(c);
	if (bc*ac - cc * ab < 0.0)
	{
		return false;
	}
	double bb = b.dot(b);
	if (ab*bc - ac * bb < 0.0)
	{
		return false;
	}
	return true;
}

//点是否在多边形内部，包含边上
bool PointInsidePolygon(Point returnPoint, Shape collisionFace)
{
	unsigned collisionFaceSize = collisionFace.size();
	for (unsigned i = 1; i < collisionFaceSize; ++i)
	{
		if (PointInsideTriangle(collisionFace[0], collisionFace[i], collisionFace[i + 1], returnPoint))
		{
			return true;
		}
	}
	return false;
}

//若相交，结果保存在p中
bool intersectSegmentPolygon(Point startPoint, Point endPoint, Point& p, Shape collisionFace, Point prePoint)
{
	Point v = endPoint - startPoint;
	//(startPoint+v*t)·normal = collisionFace[0]·normal
	//collisionFace[0]·normal-startPoint·normal=v*t·normal
	Point _normal = collisionFace.getNormal();
	//存在精确误差，认为在一个epsilon范围内都属于t的值
	double _t = (collisionFace[0].dot(_normal) - startPoint.dot(_normal)) / (v.dot(_normal));
	double t[3] = { _t,_t + PLANETHICNESS / v.dot(_normal),_t - PLANETHICNESS / v.dot(_normal) };
	Point returnPoint;
	for (unsigned i = 0; i < 3; ++i)
	{
		returnPoint.x = startPoint.x + v.x*t[i];
		returnPoint.y = startPoint.y + v.y*t[i];
		returnPoint.z = startPoint.z + v.z*t[i];
		if (PointInsidePolygon(returnPoint, collisionFace))
		{
			if (!returnPoint.equal(prePoint))
			{
				p = returnPoint;
				return true;
			}
		}
	}
	return false;
}

bool findInnerPointInDetecFace(Shape detectFace, Shape collisionFace, Point& _p, Point prePoint)
{
	unsigned collisionFaceSize = collisionFace.size();
	for (unsigned i = 0; i < collisionFaceSize; ++i)
	{
		if (classifyPointToPlane(collisionFace[i], detectFace) == POINT_ON_POLYGON)
		{
			if (PointInsidePolygon(collisionFace[i],detectFace))
			{
				if (!collisionFace[i].equal(prePoint))
				{
					_p = collisionFace[i];
					return true;
				}
			}
		}
	}
	return false;
}

//更新原来detectFace信息，成功返回true，失败返回false
bool _update(unsigned& controlPointIndex, Point startPoint, Point endPoint, Point _p, Shape collisionFace, Shape detectFace, Shape& _shape)
{
	Point prePoint;
	if (prePoints.size() > 0)
	{
		prePoint = prePoints[prePoints.size() - 1];
	}
	if (intersectSegmentPolygon(startPoint, endPoint, _p, collisionFace, prePoint))
	{
		_shape.insert(_p, controlPointIndex++);
		prePoints.push_back(_p);
		return true;
	}
	else if (findInnerPointInDetecFace(detectFace, collisionFace, _p, prePoint))
	{
		_shape.insert(_p, controlPointIndex++);
		prePoints.push_back(_p);
		return true;
	}
	else
	{
		return false;
	}
}

//与detectFace发生碰撞的面只有一个，检测是否属于“合法”的碰撞。若“合法”，则调用_update函数更新原来面的信息
bool updateShapeByOneCollisionFace(FbxMesh* lMesh,Shape& detectFace, Shape collisionFace)
{
	Shape _shape;
	unsigned detectFaceSize = detectFace.size();
	std::vector<int>intersect(detectFaceSize);
	std::vector<int>pointPosition(4, 0);
	unsigned controlPointIndex = lMesh->GetControlPointsCount(), tempControlPointIndex = controlPointIndex;//需要记录点在mesh中的位置
	prePoints.clear();//清空所有数据
	for (unsigned i = 0; i < detectFaceSize; ++i)
	{
		intersect[i] = classifyPointToPlane(detectFace[i], collisionFace);
		++pointPosition[intersect[i]];
	}
	//detectFace全在collisionFace一侧或是在collisionFace面上时
	if (pointPosition[POINT_ON_POLYGON] != 0 || (pointPosition[POINT_BEHIND_POLYGON] & pointPosition[POINT_INFRONT_POLYGON]) == 0)
	{
		return false;
	}
	//只处理简单情况，不考虑复杂的
	for (unsigned i = 0; i < detectFaceSize; ++i)
	{
		if (intersect[i] == POINT_BEHIND_POLYGON)
		{
			Point startPoint, endPoint, _p;
			if (intersect[(i - 1) % detectFaceSize] == POINT_INFRONT_POLYGON)
			{
				startPoint = detectFace[(i - 1) % detectFaceSize];
				endPoint = detectFace[i];
				if (!_update(tempControlPointIndex, startPoint, endPoint, _p, collisionFace, detectFace, _shape))
				{
					return false;
				}
			}
			if (intersect[(i + 1) % detectFaceSize] == POINT_INFRONT_POLYGON)
			{
				endPoint = detectFace[(i + 1) % detectFaceSize];
				startPoint = detectFace[i];
				if (!_update(tempControlPointIndex, startPoint, endPoint, _p, collisionFace, detectFace, _shape))
				{
					return false;
				}
			}
		}
		else
		{
			_shape.insert(detectFace[i],detectFace.getVertexAt(i));
		}
	}
	//将产生的分割点保存到mesh中
	for (unsigned i = controlPointIndex; i < tempControlPointIndex; ++i)
	{
		FbxVector4 newPoint;
		for (unsigned j=0;j<_shape.size();++j)
		{
			if (i == _shape.getVertexAt(j))
			{
				newPoint[0] = _shape[j].x;
				newPoint[1] = _shape[j].y;
				newPoint[2] = _shape[j].z;
				break;
			}
		}
		lMesh->SetControlPointAt(newPoint, i);
	}
	_shape.setNormal(detectFace.getNormal());
	detectFace = _shape;
	return true;
}

//碰撞检测
void collision(std::vector<FbxMesh*>&meshVector,std::vector<Shape>&shapes, std::vector<int>&isChanged)
{
	unsigned shapeCount = shapes.size();
	for (unsigned i = 0; i < shapeCount; ++i)
	{
		Shape expansionShape = shapes[i].expansion();
		std::vector<unsigned>onShape;
		if (i==32)
		{
			int ss = 0;
		}
		for (unsigned j = i + 1; j < shapeCount; ++j)
		{
			if (gjk(expansionShape,shapes[j]))//collision
			{
				if (!neighborShape(shapes[i],shapes[j]))//point在边上
				{
					onShape.push_back(j);
				}
			}
		}
		unsigned count = onShape.size();
		int meshID;
		for (unsigned m = meshVector.size() - 1; m >= 0; --m)
		{
			if (i >= offSet[m])
			{
				meshID = m;
				break;
			}
		}
		if (count == 0)//说明没有相交
		{
			continue;
		}else if (count == 1)//说明只有一个面相交，考虑两种情况
		{
			if (updateShapeByOneCollisionFace(meshVector[meshID],shapes[i], shapes[onShape[0]]))
			{
				isChanged[i] = 1;
			}
		}
		else//与多个面相交，采取另外一种措施
		{
			for (unsigned onShapeIndex = 0; onShapeIndex < onShape.size(); ++onShapeIndex)
			{
				Shape _shape = shapes[i];
				if (updateShapeByOneCollisionFace(meshVector[meshID],_shape, shapes[onShape[onShapeIndex]]))
				{
					isChanged[onShapeIndex] = 1;
					if (additionPolygon.find(onShapeIndex) == additionPolygon.end())
					{
						std::vector<Shape>vp;
						vp.push_back(_shape);
						additionPolygon.insert(std::make_pair(onShapeIndex, vp));
					}
					else
					{
						additionPolygon[onShapeIndex].push_back(_shape);
					}
				}
			}
		}
	}
}

//膨胀操作
void expansion(std::vector<FbxMesh*>meshVector)
{
	int numPolygon = 0;
	unsigned numMesh = meshVector.size();
	for (unsigned i = 0; i < numMesh; ++i)
	{
		offSet.push_back(numPolygon);
		numPolygon += meshVector[i]->GetPolygonCount();
	}
	std::vector<Shape>shapes(numPolygon);
	std::vector<int>isChanged(numPolygon, -1);//-1表示没改变，1表示该边
	convertMeshPolygonToShape(meshVector, shapes);
	collision(meshVector, shapes, isChanged);
	for (int i = numPolygon - 1; i >= 0; --i)
	{
		if (isChanged[i] == 1)
		{
			int meshID, polygonID;
			for (int j = numMesh - 1; j >= 0; --j)
			{
				if (i >= offSet[j])
				{
					meshID = j;
					polygonID = i - offSet[j];
					break;
				}
			}
			if (additionPolygon.find(i) == additionPolygon.end())//没有多余的多边形添加
			{
				Shape _shape = shapes[i];
				meshVector[meshID]->BeginPolygon();
				for (int j = 0; j < _shape.size(); ++j)
				{
					meshVector[meshID]->AddPolygon(_shape.getVertexAt(j));
				}
				meshVector[meshID]->EndPolygon();
			}
			else//可能要添加好几个多边形
			{
				std::vector<Shape>_polygon = additionPolygon[i];
				for (int it = 0;it<_polygon.size();++it)
				{
					Shape _shape = _polygon[it];
					meshVector[meshID]->BeginPolygon();
					for (int j = 0; j < _shape.size(); ++j)
					{
						meshVector[meshID]->AddPolygon(_shape.getVertexAt(j));
					}
					meshVector[meshID]->EndPolygon();
				}
			}
			meshVector[meshID]->RemovePolygon(i);
		}
	}
}

void processNode(FbxNode* _pNode)
{
	//只处理meshNode
	std::queue<FbxNode*>que;
	que.push(_pNode);
	std::vector<FbxMesh*>meshVector;
	while (!que.empty())
	{
		FbxNode* queNode = que.front();
		que.pop();
		if (queNode->GetNodeAttribute())
		{
			switch (queNode->GetNodeAttribute()->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
				//暂时删除！！
				//meshRepair(queNode);
				meshVector.push_back(queNode->GetMesh());
				break;
			case FbxNodeAttribute::eLight:
				FBXSDK_printf("load LightNode\n");
				break;
			case  FbxNodeAttribute::eCamera:
				FBXSDK_printf("load CameraNode\n");
				break;
			default:
				break;
			}
		}
		//GetChildCount(param)默认为false，如果为true，则会迭代计算后续孩子的个数
		//迭代访问所有的node
		int NumNode = queNode->GetChildCount();
		for (int i = 0 ; i < NumNode ; ++i)
		{
			que.push(queNode->GetChild(i));
		}
	}
	expansion(meshVector);
}

int main()
{
	FbxString lFilePath("weixch_sceneTest17_0.fbx");
	//FbxString lFilePath("../first_fbx/test_0.fbx");
	//FbxString lFilePath("chenweixing_sceneTest17_0.fbx");
	FbxManager* lManager = NULL;
	FbxScene* lScene = NULL;
	InitializeSdkObjects(lManager, lScene);

	/*FbxManager* lManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(lManager, IOSROOT);
	lManager->SetIOSettings(ios);
	FbxScene* lScene = FbxScene::Create(lManager, "MyScene");*/

	bool lResult = LoadScene(lManager, lScene, lFilePath.Buffer());
	if (lResult)
	{
		FbxNode* rootNode = lScene->GetRootNode();
		processNode(rootNode);
	}
	else {
		FBXSDK_printf("Call to LoadScene() failed.\n");
	}
	const char* lFilename = "removeHiddenFace.fbx";
	FbxExporter* lExporter = FbxExporter::Create(lManager, "");
	bool lExportStatus = lExporter->Initialize(lFilename, -1, lManager->GetIOSettings());
	if (!lExportStatus) {
		printf("Call to FbxExporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		return -1;
	}
	lExporter->Export(lScene);
	lExporter->Destroy();
	lManager->Destroy();
	return 0;
}
