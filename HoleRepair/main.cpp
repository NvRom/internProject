#include "stdafx.h"
#include <queue>

void processNode(FbxNode* _pNode)
{
	//只处理meshNode
	if (_pNode->GetNodeAttribute())
	{
		switch (_pNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			//暂时删除！！
			holeRepair(_pNode);
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
	int NumNode = _pNode->GetChildCount();
	for (int i = 0; i < NumNode; ++i)
	{
		processNode(_pNode->GetChild(i));
	}
}

int main()
{
	FbxString lFilePath("../first_fbx/sceneTest17_0.fbx");
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
	const char* lFilename = "weixch_sceneTest17_0.fbx";
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