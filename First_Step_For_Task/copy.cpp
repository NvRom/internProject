// First_Step_For_Task.cpp: �������̨Ӧ�ó������ڵ㡣
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

//��edges���ҵ���vertex�����ӵĵ��,���Ҳ����򷵻�-1
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
	//˵��ԭʼedges�д��ڶ��Ȧ�ӹ���һ��������
	return -1;
}

//����õ��պϵ�Ȧ�ӣ�����
void solveCircles(vector<vector<int>>&circles, vector<int>&circleCode)
{
	int startJ = 0;
	//��ΪcircleCode���鲻��,����ÿ�ζ��ж��Ƿ���֮ǰ�Ѿ������˵�ǰֵ
	for (int i = 0; i < circleCode.size(); ++i)
	{
		if (circleCode[i] == -1)
		{
			//��֮ǰ��ȫ��-1,�൱������startJ��ֵ
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

//����label��Ϣ������solveCircles���������ز�ͬ��Ȧ��(�����Ϣ������λ����ʾ)
vector<vector<int>> getVertexCircle(const map<int, set<pair<int, int>>>&edgeCircles)
{
	vector<vector<int>>circles;
	for (map<int, set<pair<int, int>>>::const_iterator iter = edgeCircles.begin(); iter != edgeCircles.end(); ++iter)
	{
		set<pair<int, int>>edges = iter->second;
		//edges����������ɸ�Ȧ��
		//TO DO

		vector<int>circleCode;
		circleCode.push_back(edges.begin()->first);
		circleCode.push_back(edges.begin()->second);
		edges.erase(edges.begin());
		while (!edges.empty())
		{
			int flag = findNextConnectedVertex(edges, circleCode.back());
			if (flag == -1)//˵��ʣ�µ�edges��ԭ��edges���Ӽ�����ӵ�ж�����Ȧ�ӣ�����
			{
				circleCode.push_back(-1);//������ʱ������-1����Ҫ��ǰ���ȫ������
				circleCode.push_back(edges.begin()->first);
				circleCode.push_back(edges.begin()->second);
				edges.erase(edges.begin());
			}
			else
			{
				circleCode.push_back(flag);
			}
		}
		//����circle���õ����Ȧ�ӣ�����
		solveCircles(circles, circleCode);
	}
	return circles;
}

//�����߶μ��ϣ����ú���getVertexCircle�ҵ�����һ��Ȧ�ӣ��������߶�(����)
//�ⲿ�ֺ�ʱͦ���
vector<vector<int>> processEdgeSetWithVertex(const set<pair<int, int>>&edgeSet, int vertexCount)
{
	//�ҵ�ͬ����һ������߶Σ�or�㣩
	int label = 0;
	//-1��ʾ�õ�û�з���lable
	//��edgeҲ��label�ֺá�������ô����ԭ��
	//���ֻ��¼�㣬��һ��Ȧ������ڶ�����ʱ����Ҫ�ж���Щ������ԭͼ�е���Щedge��ɵģ��Ȳ�˵�Ƿ�����ȷ�Ƶ���edge���Ƶ��Ĺ���Ҳʮ�ַ���
	vector<int>id(vertexCount, -1);
	map<int, set<pair<int, int>>>edgeCircles;
	for (set<pair<int, int>>::const_iterator iter = edgeSet.begin(); iter != edgeSet.end(); ++iter)
	{
		int p = iter->first, q = iter->second;//p<q
											  //˵�������㶼û��Ȧ�����ʱҪ�½�һ��Ȧ��;
											  //ͬʱҲҪ��edge����map��
		if (id[p] == -1 && id[q] == -1)
		{
			id[p] = id[q] = ++label;
			set<pair<int, int>>_set;
			_set.insert(*iter);
			edgeCircles.insert(make_pair(label, _set));
		}
		else if (id[p] * id[q]< 0)//˵��һ������Ȧ�ӣ���һ��������Ȧ�ӣ���ʱҪ����Ȧ����
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
		else if (id[p] != id[q])//˵��������������Ȧ�ӣ���Ȧ�ӻ���һ������ʱҪ����Ȧ��
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
			//��edgeCircles[label_q]���������ȫ��ת�Ƶ�edgeCircles[label_p]��;Ȼ��ɾ����label_q���߶�
			for (set<pair<int, int>>::iterator it = edgeCircles[label_q].begin(); it != edgeCircles[label_q].end(); ++it)
			{
				edgeCircles[label_p].insert(*it);
			}
			edgeCircles.erase(label_q);
		}
		else//˵������������һ��Ȧ�ӣ��õ�һ���պϵ�Ȧ��,
		{
			//TO DO
			edgeCircles[id[p]].insert(*iter);
		}

	}
	return getVertexCircle(edgeCircles);
}

//�õ�δƥ���edge���ϣ��Ƚ�������Ҫ��ϵ�edge
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
		//��������ε����ж��㣬д��vector��
		for (int j = 0; j < polygonSize; ++j)
		{
			indexOfPolygonVertex[j] = lMesh->GetPolygonVertex(i, j);
		}
		//����set���ϣ���ʾδ�ɹ�ƥ����߶Σ���Ϊ�����������ȷ��һ��һ���߶�
		//�߶νṹ��vertex����ֵ��С����С����ǰ��
		for (int j = 0; j < polygonSize; ++j)
		{
			int pair_first = mapping[indexOfPolygonVertex[j%polygonSize]];
			int pair_second = mapping[indexOfPolygonVertex[(j + 1) % polygonSize]];
			if (pair_first == pair_second)//��Ϊ�棬˵���ƻ���ԭģ�͵Ķ���ι�ϵ�����Ǹ�Σ�յ��źţ�
			{
				sameIndex = j;
				hasSamePoint = true;
				continue;
			}
			pair<int, int> p = make_pair(min(pair_first, pair_second), max(pair_first, pair_second));
			auto iter = edgeSet.find(p);
			if (iter == edgeSet.end())//˵�����߶β���set������,�����߶Σ�����ɾ�������߶�
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
			lMesh->BeginPolygon();//�����0,1��ʲô����
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

//�ж��������Ƿ�����
bool theNearPoint(FbxVector4 p, FbxVector4 q)
{
	return (p[0] - q[0])*(p[0] - q[0]) + (p[1] - q[1])*(p[1] - q[1]) + (p[2] - q[2])*(p[2] - q[2]) < THRESHOLD;
}
//�����ڵ����������߼�����Ϊһ����
vector<int>mappingVertex_3(const FbxVector4* controlPointsVertex, int controlPointsCount)
{
	vector<int>mapping(controlPointsCount, -1);
	map<double, int>hashMap;
	const double threshold = 0.0009; //д����
	for (int i = 0; i < controlPointsCount; ++i)
	{
		FBXSDK_printf("�����%d��{%f,%f,%f}\n", i, controlPointsVertex[i][0], controlPointsVertex[i][1], controlPointsVertex[i][2]);
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
		if (mapping[iter_i->second] == -1)//˵���õ�û�б�ӳ�䣬��Ҫ����
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

//���������Σ������䣩
void processAddPolygon(FbxMesh* lMesh, vector<vector<int>> circles)
{
	unsigned circleSize = circles.size();
	for (int i = 0; i < circleSize; ++i)
	{
		int first = circles[i][0];
		int second = circles[i][1];
		for (int j = 2; j < circles[i].size(); ++j)
		{
			lMesh->BeginPolygon();//�����0,1��ʲô����
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

//����meshNode������δƥ����߶�������ɫ
void processMesh(FbxNode* _meshNode)
{
	FbxMesh* lMesh = _meshNode->GetMesh();
	//��node����meshʱ���Ż��������Ĳ���
	if (lMesh == nullptr)
	{
		return;
	}
	FbxVector4* controlPointsVertex = lMesh->GetControlPoints();
	int controlPointsCount = lMesh->GetControlPointsCount();
	//��Ϊ������Щ��ľ���ǳ�����������Ϊ��ͬһ���㣬������Ҫ������ӳ����ƣ���ģ���еľ�������ĵ��ڴ���ʱ������һ����
	//mapping[i]=j��ʾ���ڵ�iӳ��Ϊj�����������ڵ�iʱ����Ҫ������j
	vector<int>mapping = mappingVertex_3(controlPointsVertex, controlPointsCount);
	//fix3DModel(mapping, lMesh);
	//�������е�ͼ�Σ��õ�δƥ���߶μ���
	set<pair<int, int>>edgeSet = getSingleEdgeSet(lMesh, mapping);
	////�õ���������Ҫ���Ȧ�ӣ�����
	vector<vector<int>> circles = processEdgeSetWithVertex(edgeSet, lMesh->GetControlPointsCount());
	processAddPolygon(lMesh, circles);
}

//����������
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
	//ֻ����meshNode
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
				//��ʱɾ������
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
		//GetChildCount(param)Ĭ��Ϊfalse�����Ϊtrue������������������ӵĸ���
		//�����������е�node
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