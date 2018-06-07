// First_Step_For_Task.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <fbxsdk.h>
#include "Common.h"
#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <cmath>
#include <queue>
#include "assert.h"

//在edges中找到与vertex相连接的点的,若找不到则返回-1
int findNextConnectedVertex(set<pair<int, int>>&edges, int vertex) {
	for (set<pair<int, int>>::iterator it = edges.begin(); it != edges.end(); ++it)
	{
		if (it->first == vertex || it->second == vertex)
		{
			int p = it->first == vertex ? it->second : it->first;
			edges.erase(it);
			return p;
		}
	}
	//说明原始edges中存在多个圈子共享一个点的情况
	return -1;
}

//计算得到闭合的圈子（环）
void solveCircles(vector<vector<int>>&circles, vector<int>&circleCode)
{
	int startJ = 0;
	//因为circleCode数组不大,可以每次都判断是否在之前已经访问了当前值
	for (int i = 0; i < circleCode.size(); ++i)
	{
		if (circleCode[i] == -1)
		{
			//将之前的全置-1,相当于重置startJ的值
			startJ = i + 1;
		}
		else
		{
			int j;
			for (j = startJ; j < i; ++j)
			{
				if (circleCode[j] == circleCode[i])
				{
					vector<int>_circle;
					for (; j < i; ++j)
					{
						if (circleCode[j] != -1)
						{
							_circle.push_back(circleCode[j]);
							circleCode[j] = -1;
						}
					}
					if (_circle.size() <= 10)
					{
						circles.push_back(_circle);
					}
					break;
				}
			}
		}
	}
}

//根据label信息，调用solveCircles函数，返回不同的圈子(点的信息按坐标位置提示)
vector<vector<int>> getVertexCircle(const map<int, set<pair<int, int>>>&edgeCircles)
{
	vector<vector<int>>circles;
	for (map<int, set<pair<int, int>>>::const_iterator iter = edgeCircles.begin(); iter != edgeCircles.end(); ++iter)
	{
		set<pair<int, int>>edges = iter->second;
		//edges必须包含若干个圈子
		//TO DO

		vector<int>circleCode;
		circleCode.push_back(edges.begin()->first);
		circleCode.push_back(edges.begin()->second);
		edges.erase(edges.begin());
		while (!edges.empty())
		{
			int flag = findNextConnectedVertex(edges, circleCode.back());
			if (flag == -1)//说明剩下的edges是原来edges的子集，且拥有独立的圈子（环）
			{
				circleCode.push_back(-1);//解析的时候遇到-1，需要将前面的全部归零
				circleCode.push_back(edges.begin()->first);
				circleCode.push_back(edges.begin()->second);
				edges.erase(edges.begin());
			}
			else
			{
				circleCode.push_back(flag);
			}
		}
		//解析circle，得到多个圈子（环）
		solveCircles(circles, circleCode);
	}
	return circles;
}

//处理线段集合，调用函数getVertexCircle找到属于一个圈子（环）的线段(顶点)
//这部分耗时挺多的
vector<vector<int>> processEdgeSetWithVertex(const set<pair<int, int>>&edgeSet, int vertexCount)
{
	//找到同属于一个域的线段（or点）
	int label = 0;
	//-1表示该点没有分配lable
	//将edge也按label分好。采用这么做的原因：
	//如果只记录点，当一个圈子里存在多个点的时候，需要判断这些点是由原图中的哪些edge组成的；先不说是否能正确推导出edge，推导的过程也十分繁琐
	vector<int>id(vertexCount, -1);
	map<int, set<pair<int, int>>>edgeCircles;
	for (set<pair<int, int>>::const_iterator iter = edgeSet.begin(); iter != edgeSet.end(); ++iter)
	{
		int p = iter->first, q = iter->second;//p<q
											  //说明两个点都没在圈子里，这时要新建一个圈子;
											  //同时也要把edge加入map中
		if (id[p] == -1 && id[q] == -1)
		{
			id[p] = id[q] = ++label;
			set<pair<int, int>>_set;
			_set.insert(*iter);
			edgeCircles.insert(make_pair(label, _set));
		}
		else if (id[p] * id[q]< 0)//说明一个属于圈子，另一个不属于圈子，这时要加入圈子中
		{
			if (id[p]> 0)
			{
				id[q] = id[p];
				edgeCircles[id[q]].insert(*iter);
			}
			else//id[q]>0
			{
				id[p] = id[q];
				edgeCircles[id[q]].insert(*iter);
			}
		}
		else if (id[p] != id[q])//说明两个都曾属于圈子，且圈子还不一样，这时要更新圈子
		{
			int label_p = id[p];
			int label_q = id[q];
			edgeCircles[label_p].insert(*iter);
			for (int i = 0; i < vertexCount; ++i)
			{
				if (id[i] == label_q)
				{
					id[i] = label_p;
				}
			}
			//将edgeCircles[label_q]里面的数据全都转移到edgeCircles[label_p]中;然后删除掉label_q的线段
			for (set<pair<int, int>>::iterator it = edgeCircles[label_q].begin(); it != edgeCircles[label_q].end(); ++it)
			{
				edgeCircles[label_p].insert(*it);
			}
			edgeCircles.erase(label_q);
		}
		else//说明两个都属于一个圈子，得到一个闭合的圈子,
		{
			//TO DO
			edgeCircles[id[p]].insert(*iter);
		}

	}
	return getVertexCircle(edgeCircles);
}

//得到未匹配的edge集合，既接下来需要缝合的edge
set<pair<int, int>>getSingleEdgeSet(FbxMesh* &lMesh, vector<int>&mapping)
{
	//FbxGeometryElementUV* pVertexUV = lMesh->GetElementUV(0);
	set<pair<int, int>>edgeSet;
	int polygonCount = lMesh->GetPolygonCount();
	for (int i = 0, totalIndex = 0; totalIndex < polygonCount; ++i, ++totalIndex)
	{
		bool hasSamePoint = false;
		int sameIndex = 0;
		int polygonSize = lMesh->GetPolygonSize(i);
		vector<int>indexOfPolygonVertex(polygonSize);
		//遍历多边形的所有顶点，写入vector中
		for (int j = 0; j < polygonSize; ++j)
		{
			indexOfPolygonVertex[j] = lMesh->GetPolygonVertex(i, j);
		}
		//构造set集合，表示未成功匹配的线段，因为两个顶点可以确定一个一条线段
		//线段结构按vertex索引值大小排序，小的在前面
		for (int j = 0; j < polygonSize; ++j)
		{
			int pair_first = mapping[indexOfPolygonVertex[j%polygonSize]];
			int pair_second = mapping[indexOfPolygonVertex[(j + 1) % polygonSize]];
			if (pair_first == pair_second)//若为真，说明破坏了原模型的多边形关系，这是个危险的信号！
			{
				sameIndex = j;
				hasSamePoint = true;
				continue;
			}
			pair<int, int> p = make_pair(min(pair_first, pair_second), max(pair_first, pair_second));
			auto iter = edgeSet.find(p);
			if (iter == edgeSet.end())//说明该线段不在set即合里,添加线段；否则删除掉此线段
			{
				edgeSet.insert(p);
			}
			else
			{
				edgeSet.erase(p);
			}
		}
		if (hasSamePoint)
		{
			lMesh->BeginPolygon();//这里的0,1有什么作用
			for (int j = 0; j < polygonSize; ++j)
			{
				if (j != sameIndex)
				{
					lMesh->AddPolygon(indexOfPolygonVertex[j], lMesh->GetTextureUVIndex(i, j));
				}
			}
			lMesh->EndPolygon();
			lMesh->RemovePolygon(i--);
		}
	}
	lMesh->GenerateNormals();
	lMesh->BuildMeshEdgeArray();
	return edgeSet;
}

bool fix3DModel(vector<int>&mapping, FbxMesh* lMesh)
{
	int polygonCount = lMesh->GetPolygonCount();
	int totalIndex = lMesh->GetPolygonVertexCount();
	int* vertexIndex = lMesh->GetPolygonVertices();
	for (int i = 0; i < totalIndex; ++i)
	{
		vertexIndex[i] = mapping[vertexIndex[i]];
	}
	return true;
}

//判断两个点是否相邻
bool theNearPoint(FbxVector4 p, FbxVector4 q)
{
	return (p[0] - q[0])*(p[0] - q[0]) + (p[1] - q[1])*(p[1] - q[1]) + (p[2] - q[2])*(p[2] - q[2]) < THRESHOLD;
}
//将相邻的两个点在逻辑上视为一个点
vector<int>mappingVertex_3(const FbxVector4* controlPointsVertex, int controlPointsCount)
{
	vector<int>mapping(controlPointsCount, -1);
	map<double, int>hashMap;
	const double threshold = 0.0009; //写死了
	for (int i = 0; i < controlPointsCount; ++i)
	{
		FBXSDK_printf("结点编号%d：{%f,%f,%f}\n", i, controlPointsVertex[i][0], controlPointsVertex[i][1], controlPointsVertex[i][2]);
		double hashValue = controlPointsVertex[i][0] + controlPointsVertex[i][1] + controlPointsVertex[i][2];
		int sss = hashMap.count(hashValue);
		while (hashMap.count(hashValue))
		{
			hashValue += HASHVALUESTEPLENGTH;
		}
		hashMap.insert(make_pair(controlPointsVertex[i][0] + controlPointsVertex[i][1] + controlPointsVertex[i][2], i));
	}
	auto iter_i = hashMap.begin();
	for (iter_i = hashMap.begin(); iter_i != hashMap.end(); ++iter_i)
	{
		if (mapping[iter_i->second] == -1)//说明该点没有被映射，需要处理
		{
			auto iter_j = iter_i;
			++iter_j;
			if (iter_j == hashMap.end())
			{
				mapping[iter_i->second] = iter_i->second;
				break;
			}
			double subNum = iter_i->first - iter_j->first;
			bool istheNearPoint = false;
			while (subNum * subNum < threshold)
			{
				if (theNearPoint(controlPointsVertex[iter_i->second], controlPointsVertex[iter_j->second]))
				{
					istheNearPoint = true;
					if (mapping[iter_i->second] == -1)
					{
						mapping[iter_i->second] = mapping[iter_j->second] = iter_i->second < iter_j->second ? iter_i->second : iter_j->second;
					}
					else
					{
						mapping[iter_j->second] = mapping[iter_i->second];
					}
				}
				++iter_j;
				if (iter_j == hashMap.end())
				{
					break;
				}
				subNum = iter_i->first - iter_j->first;
			}
			if (!istheNearPoint)
			{
				mapping[iter_i->second] = iter_i->second;
			}
		}
	}
	return mapping;
}
//vector<int>mappingVertex_2(const FbxVector4* controlPointsVertex, int controlPointsCount)
//{
//	map<int, set<int>>bucket;
//	double min_x, min_y, min_z, max_x, max_y, max_z, step_x, step_y, step_z;
//	min_x = min_y = min_z = 10e5;
//	max_x = max_y = max_z = -10e5;
//	for (int i = 0; i < controlPointsCount; ++i)
//	{
//		min_x = min_x < controlPointsVertex[i][0] ? min_x : controlPointsVertex[i][0];
//		min_y = min_y < controlPointsVertex[i][1] ? min_y : controlPointsVertex[i][1];
//		min_z = min_z < controlPointsVertex[i][2] ? min_z : controlPointsVertex[i][2];
//		max_x = max_x > controlPointsVertex[i][0] ? max_x : controlPointsVertex[i][0];
//		max_y = max_y > controlPointsVertex[i][1] ? max_y : controlPointsVertex[i][1];
//		max_z = max_z > controlPointsVertex[i][2] ? max_z : controlPointsVertex[i][2];
//	}
//	step_z = (max_z + 0.01 - min_z) / 4;
//	step_y = (max_y + 0.01 - min_y) / 4;
//	step_x = (max_x + 0.01 - min_x) / 4;
//	vector<int>mapping(controlPointsCount, -1);
//	for (int i = 0; i < controlPointsCount; ++i)
//	{
//		int label = 16 * floor((controlPointsVertex[i][0] - min_x) / step_x) + 4 * floor((controlPointsVertex[i][1] - min_y) / step_y) + floor((controlPointsVertex[i][2] - min_z) / step_z);
//		bool istheNearPoint = false;
//		if (bucket.find(label) == bucket.end())
//		{
//			set<int>_set;
//			_set.insert(i);
//			bucket.insert(make_pair(label, _set));
//			mapping[i] = i;
//		}
//		else
//		{
//			for (auto iter = bucket[label].begin(); iter != bucket[label].end(); ++iter)
//			{
//				if (theNearPoint(controlPointsVertex[i], controlPointsVertex[mapping[*iter]]))
//				{
//					istheNearPoint = true;
//					mapping[i] = mapping[*iter];
//					break;
//				}
//			}
//			if (!istheNearPoint)
//			{
//				mapping[i] = i;
//			}
//			bucket[label].insert(i);
//		}
//	}
//	return mapping;
//}
//vector<int>mappingVertex_1(const FbxVector4* controlPointsVertex, int controlPointsCount)
//{
//	vector<int>mapping(controlPointsCount);
//	for (int i = 0; i < controlPointsCount; ++i)
//	{
//		bool istheNearPoint = false;
//		for (auto j = 0; j < i; ++j)
//		{
//			if (theNearPoint(controlPointsVertex[i], controlPointsVertex[j]))
//			{
//				istheNearPoint = true;
//				mapping[i] = mapping[j];
//				break;
//			}
//		}
//		if (!istheNearPoint)
//		{
//			mapping[i] = i;
//		}
//	}
//	return mapping;
//}

//添加三角形（待补充）
void processAddPolygon(FbxMesh* lMesh, vector<vector<int>> circles)
{
	unsigned circleSize = circles.size();
	for (int i = 0; i < circleSize; ++i)
	{
		int first = circles[i][0];
		int second = circles[i][1];
		for (int j = 2; j < circles[i].size(); ++j)
		{
			lMesh->BeginPolygon();//这里的0,1有什么作用
			lMesh->AddPolygon(first);
			lMesh->AddPolygon(second);
			lMesh->AddPolygon(circles[i][j]);
			lMesh->EndPolygon();
			second = circles[i][j];
		}
	}
	bool setsss = lMesh->GenerateNormals();
	lMesh->BuildMeshEdgeArray();
}

//处理meshNode，并将未匹配的线段区域着色
void processMesh(FbxNode* _meshNode)
{
	FbxMesh* lMesh = _meshNode->GetMesh();
	//当node属于mesh时，才会进行下面的步骤
	if (lMesh == nullptr)
	{
		return;
	}
	FbxVector4* controlPointsVertex = lMesh->GetControlPoints();
	int controlPointsCount = lMesh->GetControlPointsCount();
	//因为发现有些点的距离非常近，可以认为是同一个点，所以需要建立个映射机制，将模型中的距离相近的点在处理时看成是一个点
	//mapping[i]=j表示将节点i映射为j，程序遇到节点i时，都要看成是j
	vector<int>mapping = mappingVertex_3(controlPointsVertex, controlPointsCount);
	//fix3DModel(mapping, lMesh);
	//遍历所有的图形，得到未匹配线段集合
	set<pair<int, int>>edgeSet = getSingleEdgeSet(lMesh, mapping);
	////得到所有满足要求的圈子（环）
	vector<vector<int>> circles = processEdgeSetWithVertex(edgeSet, lMesh->GetControlPointsCount());
	processAddPolygon(lMesh, circles);
}

//消除隐藏面
void removeHiddenFace(std::vector<FbxMesh*>meshVector)
{
	int numPolygon = 0;
	unsigned numMesh = meshVector.size();
	std::vector<unsigned>offSet(numMesh);
	for (int i = 0; i < numMesh; ++i)
	{
		offSet[i] = numPolygon;
		numPolygon += meshVector[i]->GetPolygonCount();
	}
	std::vector<Polygon3D>polygons(numPolygon);

	for (int i = 0; i < numMesh; ++i)
	{
		unsigned off_set = offSet[i];
		FbxVector4* controlPointsVertex = meshVector[i]->GetControlPoints();
		int polygonCountInMesh_I = meshVector[i]->GetPolygonCount();
		for (int pPolygonIndex = 0; pPolygonIndex < polygonCountInMesh_I; ++pPolygonIndex)
		{
			int polygonVertexCount = meshVector[i]->GetPolygonSize(pPolygonIndex);
			Polygon3D _poly;
			for (int pPositionInPolygon = 0; pPositionInPolygon < polygonVertexCount; ++pPositionInPolygon)
			{
				Point3D _p(controlPointsVertex[meshVector[i]->GetPolygonVertex(pPolygonIndex, pPositionInPolygon)]);
				assert(_poly.insertPoint(_p));
			}
			polygons[off_set + pPolygonIndex] = _poly;
		}
	}
	BSPNode* root = buildBSPTree(polygons, 0);
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
				//processMesh(queNode);
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
		for (int i = 0; i < NumNode; ++i)
		{
			que.push(queNode->GetChild(i));
		}
	}
	removeHiddenFace(meshVector);
}

//int main()
//{
//	FbxString lFilePath("../first_fbx/sceneTest17_0.fbx");
//	//FbxString lFilePath("../first_fbx/test_0.fbx");
//	//FbxString lFilePath("chenweixing_sceneTest17_0.fbx");
//	FbxManager* lManager = NULL;
//	FbxScene* lScene = NULL;
//	InitializeSdkObjects(lManager, lScene);
//
//	/*FbxManager* lManager = FbxManager::Create();
//	FbxIOSettings* ios = FbxIOSettings::Create(lManager, IOSROOT);
//	lManager->SetIOSettings(ios);
//	FbxScene* lScene = FbxScene::Create(lManager, "MyScene");*/
//
//	bool lResult = LoadScene(lManager, lScene, lFilePath.Buffer());
//	if (lResult)
//	{
//		FbxNode* rootNode = lScene->GetRootNode();
//		processNode(rootNode);
//	}
//	else {
//		FBXSDK_printf("Call to LoadScene() failed.\n");
//	}
//	const char* lFilename = "weixch_sceneTest17_0.fbx";
//	FbxExporter* lExporter = FbxExporter::Create(lManager, "");
//	bool lExportStatus = lExporter->Initialize(lFilename, -1, lManager->GetIOSettings());
//	if (!lExportStatus) {
//		printf("Call to FbxExporter::Initialize() failed.\n");
//		printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
//		return -1;
//	}
//	lExporter->Export(lScene);
//	lExporter->Destroy();
//	lManager->Destroy();
//	return 0;
//}