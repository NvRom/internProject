#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include <fbxsdk.h>
#include <queue>
#include <set>
#include <vector>
#include <iostream>
#include "Common.h"

using namespace std;
//模型的一些属性
vector<FbxMesh*>meshVector;
vector<unsigned int>offSet;
set<unsigned int>visibleFace;
vector<vector<unsigned int>>visiableFace;
double max_x = -DBL_MAX, min_x = DBL_MAX, max_y = -DBL_MAX, min_y = DBL_MAX, max_z = -DBL_MAX, min_z = DBL_MAX;

//屏幕长宽
const unsigned int SCR_WIDTH = 2400;
const unsigned int SCR_HEIGHT = 1800;

//相机属性
Camera camera(glm::vec3(-0.5f, 0.0f, 4.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

//对fbx中的每一个面进行一次query操作。如果query结果为可见，则将此面id保存在visibleFace中
void queryObject(vector<FbxMesh*>meshVector, Shader queryShader)
{
	unsigned int meshVectorCount = meshVector.size();
	unsigned *query = new unsigned[offSet[offSet.size() - 1]];
	glGenQueries(offSet[meshVectorCount], query);
	for (unsigned int meshID = 0; meshID < meshVectorCount; ++meshID)
	{
		FbxMesh*_pNode = meshVector[meshID];
		int polygonCount = _pNode->GetPolygonCount();
		FbxVector4* controlPoints = _pNode->GetControlPoints();
		for (int i = 0; i < polygonCount; ++i)
		{
			int polygonVertexCount = _pNode->GetPolygonSize(i);
			vector<Vertex>vertices(polygonVertexCount);
			vector<unsigned int>verticesIndex(polygonVertexCount);
			vector<unsigned int>indices;
			for (int j = 0; j < polygonVertexCount; ++j)
			{
				verticesIndex[j] = _pNode->GetPolygonVertex(i, j);
				FbxVector4 vertex = controlPoints[verticesIndex[j]];
				Vertex _v(vertex[0], vertex[1], vertex[2]);
				vertices[j] = _v;
			}
			for (int j = 1; j < polygonVertexCount - 1; ++j)
			{
				indices.push_back(0);
				indices.push_back(j);
				indices.push_back(j + 1);
			}
			Mesh mesh(vertices, indices);
			glBeginQuery(GL_SAMPLES_PASSED, query[i + offSet[meshID]]);
			mesh.Draw(queryShader);
			glEndQuery(GL_SAMPLES_PASSED);

		}
		for (int i = polygonCount - 1; i >= 0; --i)
		{
			GLboolean occluded = GL_FALSE;
			GLint passingSamples;
			glGetQueryObjectiv(query[i + offSet[meshID]], GL_QUERY_RESULT, &passingSamples);
			if (passingSamples != 0)
			{
				visibleFace.insert(i + offSet[meshID]);
			}
		}
	}
	delete[] query;
}

//得到整个模型整体，而不是部分
void getMeshVector(FbxNode* _pNode)
{
	queue<FbxNode*>que;
	que.push(_pNode);
	while (!que.empty())
	{
		FbxNode* queNode = que.front();
		que.pop();
		if (queNode->GetNodeAttribute())
		{
			switch (queNode->GetNodeAttribute()->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
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
		int NumNode = queNode->GetChildCount();
		for (int i = 0; i < NumNode; ++i)
		{
			que.push(queNode->GetChild(i));
		}
	}
}

//计算得到模型的中心点
FbxVector4 celCenterPoint(vector<FbxMesh*>meshVector)
{
	offSet.push_back(0);
	unsigned int meshCount = meshVector.size();
	for (unsigned int j = 0; j < meshCount; ++j)
	{
		FbxMesh * lMesh = meshVector[j];
		unsigned int controlPointsCount = lMesh->GetControlPointsCount();
		FbxVector4* controlPoints = lMesh->GetControlPoints();
		offSet.push_back(offSet.back() + lMesh->GetPolygonCount());
		for (unsigned int i = 0; i < controlPointsCount; ++i)
		{
			FbxVector4 vertex = controlPoints[i];
			max_x = max_x < vertex[0] ? vertex[0] : max_x;
			min_x = min_x > vertex[0] ? vertex[0] : min_x;
			max_y = max_y < vertex[1] ? vertex[1] : max_y;
			min_y = min_y > vertex[1] ? vertex[1] : min_y;
			max_z = max_z < vertex[2] ? vertex[2] : max_z;
			min_z = min_z > vertex[2] ? vertex[2] : min_z;
		}
	}
	FbxVector4 returnValue;

	returnValue[0] = (max_x + min_x) / 2;
	returnValue[1] = (max_y + min_y) / 2;
	returnValue[2] = (max_z + min_z) / 2;
	return returnValue;
}

//根据query，删除隐藏面
void removeHiddenFace(vector<unsigned>unvisibleFace, vector<FbxMesh*>meshVector)
{
	unsigned unvisibleFaceCount = unvisibleFace.size();
	unsigned offsetCount = offSet.size();
	for (int i = unvisibleFaceCount - 1; i >= 0; --i)
	{
		unsigned meshPolygonID, meshID;
		for (int j = offsetCount - 1; j >= 0; --j)
		{
			if (unvisibleFace[i] >= offSet[j])
			{
				meshID = j;
				meshPolygonID = unvisibleFace[i] - offSet[j];
				break;
			}
		}
		meshVector[meshID]->RemovePolygon(meshPolygonID);
	}
}

//RSF：隐藏面剔除
bool RSF(FbxString modelPath, GLFWwindow* window)
{
	Shader ourShader("modelLoading.vs", "modelLoading.fs");
	FbxString lFilePath(modelPath);
	FbxManager* lManager = NULL;
	FbxScene* lScene = NULL;
	InitializeSdkObjects(lManager, lScene);
	bool lResult = LoadScene(lManager, lScene, lFilePath.Buffer());
	FbxNode* rootNode;
	FbxVector4 center;
	if (lResult)
	{
		rootNode = lScene->GetRootNode();
		getMeshVector(rootNode);
		center = celCenterPoint(meshVector);
	}
	else {
		FBXSDK_printf("Call to LoadScene() failed.\n");
		return false;
	}
	//相机位置
	double cameraPosition[36]{
		2 * min_x - center[0],min_y,2 * min_z - center[2],
		2 * max_x - center[0],min_y,2 * min_z - center[2],
		2 * min_x - center[0],min_y,2 * max_z - center[2],
		2 * max_x - center[0],min_y,2 * max_z - center[2],

		2 * min_x - center[0],center[1],2 * min_z - center[2],
		2 * max_x - center[0],center[1],2 * min_z - center[2],
		2 * min_x - center[0],center[1],2 * max_z - center[2],
		2 * max_x - center[0],center[1],2 * max_z - center[2],

		2 * min_x - center[0],2 * max_y - center[1],2 * min_z - center[2],
		2 * max_x - center[0],2 * max_y - center[1],2 * min_z - center[2],
		2 * min_x - center[0],2 * max_y - center[1],2 * max_z - center[2],
		2 * max_x - center[0],2 * max_y - center[1],2 * max_z - center[2],
	};
	//迭代渲染12次，因为相机观测值为12
	unsigned int loopCount = 0;
	while (!glfwWindowShouldClose(window) && loopCount++ < 12)
	{
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ourShader.use();

		Camera _camera(glm::vec3(cameraPosition[3 * loopCount + 0], cameraPosition[3 * loopCount + 1], cameraPosition[3 * loopCount + 2]));
		camera = _camera;

		//设置渲染器属性
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix(glm::vec3(center[0], center[1], center[2]));
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		glm::mat4 model;
		ourShader.setMat4("model", model);
		//query模型
		queryObject(meshVector, ourShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	vector<unsigned>unvisibleFace;
	unsigned i = 0;
	for (set<unsigned>::iterator it = visibleFace.begin(); it != visibleFace.end(); ++it, ++i)
	{
		while (*it != i)
		{
			unvisibleFace.push_back(i);
			++i;
		}
	}
	removeHiddenFace(unvisibleFace, meshVector);
	const char* lFilename = "RSF.fbx";
	FbxExporter* lExporter = FbxExporter::Create(lManager, "");
	bool lExportStatus = lExporter->Initialize(lFilename, -1, lManager->GetIOSettings());
	lExporter->Export(lScene);
	lExporter->Destroy();
	lManager->Destroy();
	return true;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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