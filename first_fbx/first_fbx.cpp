// first_fbx.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fbxsdk.h>
#include "Common.h"
#include <iostream>
#include <set>
#include <vector>

using namespace std;

const double THRESHOLD = 0.0002;

bool theNearPoint(FbxVector4 p, FbxVector4 q)
{
	return (p[0] - q[0])*(p[0] - q[0]) + (p[1] - q[1])*(p[1] - q[1]) + (p[2] - q[2])*(p[2] - q[2]) < THRESHOLD;
}

vector<int>mappingVertex_1(const FbxVector4* controlPointsVertex, int controlPointsCount)
{
	vector<int>mapping(controlPointsCount);
	for (int i = 0; i < controlPointsCount; ++i)
	{
		bool istheNearPoint = false;
		for (auto j = 0; j < i; ++j)
		{
			if (theNearPoint(controlPointsVertex[i], controlPointsVertex[j]))
			{
				istheNearPoint = true;
				mapping[i] = mapping[j];
				break;
			}
		}
		if (!istheNearPoint)
		{
			mapping[i] = i;
		}
	}
	return mapping;
}

void processMesh(FbxNode* _meshNode) 
{
	FbxMesh* meshNode = _meshNode->GetMesh();
	//当node属于mesh时，才会进行下面的步骤
	if (meshNode == nullptr)
	{
		return;
	}
	FbxVector4 startPoint(0, 0, -50);
	FbxVector4 endPoint(0, 0, 50);
	int vertexSize = meshNode->GetControlPointsCount();
	meshNode->SetControlPointAt(startPoint, vertexSize);
	meshNode->SetControlPointAt(endPoint, vertexSize+1);
	int edgeCount = meshNode->GetMeshEdgeCount();
	meshNode->BeginAddMeshEdgeIndex();
	int i = meshNode->AddMeshEdgeIndex(vertexSize, vertexSize + 1, true);
	int j = meshNode->AddMeshEdgeIndex(4, vertexSize + 1, true);
	int m = meshNode->AddMeshEdgeIndex(vertexSize, 4, true);
	meshNode->EndAddMeshEdgeIndex();
	edgeCount = meshNode->GetMeshEdgeCount();
	int polygonSize = meshNode->GetPolygonCount();
	for (int i = 0; i < polygonSize; ++i)
	{
		vertexSize = meshNode->GetPolygonSize(i);
		for (int j = 0; j < vertexSize; ++j)
		{
			std::cout << meshNode->GetPolygonVertex(i, j) << "\t";
		}
		std::cout << "\n";
	}
}

void processNode(FbxNode* _pNode) 
{
	if (_pNode->GetNodeAttribute())
	{
		switch (_pNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh :
			processMesh(_pNode);
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
	for (int i = 0 ; i < _pNode->GetChildCount() ; ++ i)
	{
		processNode(_pNode->GetChild(i));
	}
}
//
//int main()
//{
//	FbxString lFilePath("chenweixing_Lambert.fbx");
//	FbxString lFilePath("sceneTest17_0.fbx");
//	FbxManager* lSdkManager = NULL;
//	FbxScene* lScene = NULL;
//	InitializeSdkObjects(lSdkManager, lScene);
//	/*
//	FbxManager* lManager = FbxManager::Create();
//	FbxScene* lScene = FbxScene::Create(lManager, "");
//	*/
//	bool lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());
//	if (lResult)
//	{
//		FbxNode* rootNode = lScene->GetRootNode();
//		processNode(rootNode);
//	}
//	else {
//		FBXSDK_printf("Call to LoadScene() failed.\n");
//	}
//	const char* lFilename = "chenweixing_Lambert1.fbx";
//	FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");
//	bool lExportStatus = lExporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings());
//	if (!lExportStatus) {
//		printf("Call to FbxExporter::Initialize() failed.\n");
//		printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
//		return -1;
//	}
//	lExporter->Export(lScene);
//	lExporter->Destroy();
//	lSdkManager->Destroy();
//	return 0;
//}

void ConnectMaterialToMesh(FbxMesh* pMesh, int triangleCount, int* pTriangleMtlIndex)
{
	// Get the material index list of current mesh  
	FbxLayerElementArrayTemplate<int>* pMaterialIndices;
	FbxGeometryElement::EMappingMode   materialMappingMode = FbxGeometryElement::eNone;

	if (pMesh->GetElementMaterial())
	{
		pMaterialIndices = &pMesh->GetElementMaterial()->GetIndexArray();
		materialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
		if (pMaterialIndices)
		{
			switch (materialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				if (pMaterialIndices->GetCount() == triangleCount)
				{
					for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
					{
						int materialIndex = pMaterialIndices->GetAt(triangleIndex);

						pTriangleMtlIndex[triangleIndex] = materialIndex;
					}
				}
			}
			break;

			case FbxGeometryElement::eAllSame:
			{
				int lMaterialIndex = pMaterialIndices->GetAt(0);

				for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
				{
					int materialIndex = pMaterialIndices->GetAt(triangleIndex);

					pTriangleMtlIndex[triangleIndex] = materialIndex;
				}
			}
			}
		}
	}
}

void CreateMaterials(FbxScene* lScene , FbxMesh* lMesh){
	for (int i = 1 ; i < 5 ; ++ i)
	{
		FbxString lMaterialName = "material";
		FbxString lShadingName = "Phong";
		lMaterialName += i;
		FbxDouble3 lBlack(0.0, 0.0, 0.0);
		FbxDouble3 lRed(1.0, 0.0, 0.0);
		FbxDouble3 lColor;
		FbxSurfacePhong *lMaterial = FbxSurfacePhong::Create(lMesh, lMaterialName.Buffer());


		// Generate primary and secondary colors.
		lMaterial->Emissive.Set(lBlack);
		lMaterial->Ambient.Set(lRed);
		lColor = FbxDouble3(i > 2 ? 1.0 : 0.0,
			i > 0 && i < 4 ? 1.0 : 0.0,
			i % 2 ? 0.0 : 1.0);
		lMaterial->Diffuse.Set(lColor);
		lMaterial->TransparencyFactor.Set(0.0);
		lMaterial->ShadingModel.Set(lShadingName);
		lMaterial->Shininess.Set(0.5);

		//get the node of mesh, add material for it.
		FbxNode* lNode = lMesh->GetNode();
		if (lNode)
			lNode->AddMaterial(lMaterial);
	}
}

FbxNode* getMeshNode(FbxScene* lScene) 
{
	FbxMesh* lMesh = FbxMesh::Create(lScene, "mesh");
	//定义顶点坐标
	FbxVector4 vertex0(-66.796968, 21.735851, -128.970592);
	FbxVector4 vertex1(-65.974550, 21.735851, -129.952545);
	FbxVector4 vertex2(-66.025988, 21.735851, -130.047579);
	FbxVector4 vertex3(-66.870457, 21.735851, -129.063272);

	FbxVector4 vertex4(-66.837591, 21.735849, -129.065727);
	FbxVector4 vertex5(-66.005417, 21.735849, -130.049448);
	FbxVector4 vertex6(-66.005417, 23.247467, -130.049448);
	FbxVector4 vertex7(-66.837591, 23.247467, -129.065727);
	//FbxVector4 vertex5(0, 0, -50);
	//定义顶点
	lMesh->InitControlPoints(8);
	FbxVector4* lControlPoints = lMesh->GetControlPoints();
	lControlPoints[0] = vertex0;
	lControlPoints[1] = vertex1;
	lControlPoints[2] = vertex2;
	lControlPoints[3] = vertex3;
	lControlPoints[4] = vertex4;
	lControlPoints[5] = vertex5;
	lControlPoints[6] = vertex6;
	lControlPoints[7] = vertex7;
	//lControlPoints[5] = vertex5;
	//定义多边形的面
	int lPolygonVertices[] = { 0,1,2,3,4,5,6,7};

	int newlPolygonVertices[] = { 0, 3, 2, 1,
		0, 1, 4,
		1, 2, 4,
		2, 3, 4,
		3, 0, 4};

	lMesh->BeginPolygon(0);//这里的0,1有什么作用
	lMesh->AddPolygon(lPolygonVertices[0]);
	lMesh->AddPolygon(lPolygonVertices[1]);
	lMesh->AddPolygon(lPolygonVertices[2]);
	lMesh->AddPolygon(lPolygonVertices[3]);
	lMesh->EndPolygon();
	lMesh->BeginPolygon(0);//这里的0,1有什么作用
	lMesh->AddPolygon(lPolygonVertices[4]);
	lMesh->AddPolygon(lPolygonVertices[5]);
	lMesh->AddPolygon(lPolygonVertices[6]);
	lMesh->AddPolygon(lPolygonVertices[7]);
	lMesh->EndPolygon();

	lMesh->BuildMeshEdgeArray();
	FbxNode* meshNode = FbxNode::Create(lScene, "meshNode");
	meshNode->SetNodeAttribute(lMesh);
	//CreateMaterials(lScene, lMesh);

	return meshNode;

}

//创建金字塔模型
int main() {
	FbxManager* lManager = nullptr;
	FbxScene* lScene = nullptr;
	InitializeSdkObjects(lManager, lScene);

	FbxNode* meshNode = getMeshNode(lScene);
	FbxNode* rootNode = lScene->GetRootNode();
	rootNode->AddChild(meshNode);
	const char* lFilename = "chenweixing_Lambert.fbx";
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