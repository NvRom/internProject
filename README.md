# H63 Intern Project
### 目录：
### 1. 孔洞缝合
### 2. 隐藏面分割
### 3. 隐藏面剔除

---
## 孔洞缝合
### 1. 简介
> 入口函数是holeRepair，该函数接收一个参数类型为FbxNode*的函数。
> ```
> void holeRepair(FbxNode* meshNode)
> ```
> 该函数首先会判断FbxNode类型的参数中是否包含FbxMesh属性。如果包含，会执行以下操作：
> > 1. 检查mesh中是否存在因坐标偏移而产生的缝隙。如果存在，则进行顶点焊接。
> > 2. 检查mesh中是否存在孔洞区域。如果存在，则会找到所有的孔洞区域，然后进行缝合。
> 
> 该函数执行过程中所有对mesh的修改信息都保存在输入参数meshNode里。

### 2. 使用
> 调用此接口函数，需要[FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2019-0)环境，我采用的FBX SDK版本是2019.0 VS2015。fbx文件的读写操作可以由下述代码实现：

```
    //此处为fbx文件地址，可以为相对地址，也可以是绝对地址
    FbxString lFilePath("../first_fbx/sceneTest17_0.fbx");
	FbxManager* lManager = NULL;
	FbxScene* lScene = NULL;
	InitializeSdkObjects(lManager, lScene);
	bool lResult = LoadScene(lManager, lScene, lFilePath.Buffer());
	if (lResult)
	{
		FbxNode* rootNode = lScene->GetRootNode();
		processNode(rootNode);
	}
	else {
		FBXSDK_printf("Call to LoadScene() failed.\n");
	}
```

```
    //此处为输出fbx文件名
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
```
processNode函数是递归遍历整个fbx文件的子孩子，只处理有Mesh属性的孩子节点。

```
void processNode(FbxNode* _pNode)
{
	//只处理meshNode
	if (_pNode->GetNodeAttribute())
	{
		switch (_pNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			//孔洞缝合函数
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
	int NumNode = _pNode->GetChildCount();
	for (int i = 0; i < NumNode; ++i)
	{
		processNode(_pNode->GetChild(i));
	}
}
```

---

## 隐藏面分割
### 1. 简介
入口函数是splitHiddenFace，只接受一个参数。参数类型是vector<FbxMesh*>，表示整个模型。跟holeRepair一样，函数执行期间所有的改动会保存在meshVector中。函数执行完，最终可以分割同一面片的可见/不可见部分。

```
void splitHiddenFace(std::vector<FbxMesh*>meshVector)
```
函数执行过程中主要包括三个过程：
> 1. 面片沿面发现方向移动EPSILON距离；
> 2. GJK判断两个面片是否发生了碰撞，并采用八叉树场景管理加速；
> 3. 如果发生碰撞，计算碰撞产生的线段，并根据碰撞类型分割；

### 2. 使用
函数的使用跟holeRepair类似，差别在于processNode部分，因为splitHiddenFace函数接受的参数是vector<FbxMesh*>，因此需要获取得到所有的包含mesh属性的结点。

---
## 隐藏面剔除
### 1. 简介
入口函数是RSF，接收两个参数，分别是FbxString类型的path路径，以及GLFWwindows*类型的窗口windows。函数执行过程中，会从12个角度观察模型，在每一次观察时都会进行一次query，最终得到每一次观察都不可见的面，然后删除之。
```
bool RSF(FbxString modelPath, GLFWwindow* window)
```
### 2. 使用
在使用此函数之前，需要配置相应的依赖库和include文件。具体内容现已经放在主目录下的include文件夹里。
用vs2017打开此工程文件，配置包含目录以及库目录两个选项，如下图。
> (因为我的glfw-3.2.1.bin.WIN32文件夹放在C:\Program Files下)

![image](http://ospargvdr.bkt.clouddn.com/include%20file.png)
调用RSF函数的过程可以如下图所示：

```
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RemoveHiddenFace", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	FbxString lFilePath("../First_Step_For_Task/removeHiddenFace.fbx");
    RSF(lFilePath, window);
	glfwTerminate();
	return 0;
}
```
