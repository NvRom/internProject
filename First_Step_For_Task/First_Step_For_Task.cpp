// First_Step_For_Task.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <fbxsdk.h>
#include <queue>
#include <assert.h>

//消除隐藏面
void removeHiddenFace(std::vector<FbxMesh*>meshVector)
{
	int numPolygon = 0;
	unsigned numMesh = meshVector.size();
	std::vector<unsigned>offSet(numMesh);
	for (unsigned i = 0 ; i < numMesh; ++i)
	{
		offSet[i] = numPolygon;
		numPolygon += meshVector[i]->GetPolygonCount();
	}
	std::vector<Shape>polygons(numPolygon);
	meshVector[0]->AddMeshEdgeIndexForPolygon()
	
	for (unsigned i = 0 ; i < numMesh; ++i)
	{
		unsigned off_set = offSet[i];
		FbxVector4* controlPointsVertex = meshVector[i]->GetControlPoints();
		int polygonCountInMesh_I = meshVector[i]->GetPolygonCount();
		for (int pPolygonIndex = 0 ; pPolygonIndex < polygonCountInMesh_I ; ++pPolygonIndex)
		{
			int polygonVertexCount = meshVector[i]->GetPolygonSize(pPolygonIndex);
			Polygon3D _poly;
			for (int pPositionInPolygon = 0 ; pPositionInPolygon < polygonVertexCount ; ++pPositionInPolygon)
			{
				Point3D _p(controlPointsVertex[meshVector[i]->GetPolygonVertex(pPolygonIndex, pPositionInPolygon)]);
				assert(_poly.insertPoint(_p));
			}
			polygons[off_set + pPolygonIndex] = _poly;
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
	removeHiddenFace(meshVector);
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