#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_s.h"
#include "camera.h"
#include "model.h"
#include "Common.h"

#include <fbxsdk.h>
#include<queue>
#include <iostream>

using namespace std;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void drawPolygon(vector<double> vertices, vector<unsigned int>indices, Shader queryShader)
{
	queryShader.use();
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	queryShader.setMat4("projection", projection);
	queryShader.setMat4("view", view);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	queryShader.setMat4("model", model);
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(double), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void queryObject(FbxMesh* _pNode, Shader queryShader)
{
	int polygonCount = _pNode->GetPolygonCount();
	unsigned *query = new unsigned[polygonCount];
	FbxVector4* controlPoints = _pNode->GetControlPoints();
	glGenQueries(polygonCount, query);
	//
	for (int i = 0; i < polygonCount; ++i)
	{
		int polygonVertexCount = _pNode->GetPolygonSize(i);
		vector<double>vertices(3 * polygonVertexCount);
		vector<unsigned int>indices;
		//vector<unsigned>indices;
		for (int j = 0; j < polygonVertexCount; ++j)
		{
			FbxVector4 vertex = controlPoints[_pNode->GetPolygonVertex(i, j)];
			vertices[j * 3] = vertex[0];
			vertices[j * 3 + 1] = vertex[1];
			vertices[j * 3 + 2] = vertex[2];
		}
		for (int j = 1; j < polygonVertexCount - 1; ++j)
		{
			indices.push_back(0);
			indices.push_back(j);
			indices.push_back(j + 1);
		}
		glBeginQuery(GL_SAMPLES_PASSED,query[i]);
		drawPolygon(vertices, indices, queryShader);
		glEndQuery(GL_SAMPLES_PASSED);
	}
	for (int i = polygonCount; i >= 0; --i)
	{
		GLboolean occluded = GL_FALSE;
		GLint passingSamples;
		glGetQueryObjectiv(query[i], GL_QUERY_RESULT, &passingSamples);
		if (passingSamples == 0)
		{
			//ÍêÈ«ÕÚµ²
			_pNode->RemovePolygon(i);
		}
	}
	delete query;
}


void processNode(FbxNode* _pNode,Shader queryShader)
{
	if (_pNode->GetNodeAttribute())
	{
		switch (_pNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			queryObject(_pNode->GetMesh(), queryShader);
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
		processNode(_pNode->GetChild(i), queryShader);
	}
}


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	Shader ourShader("1.model_loading.vs", "1.model_loading.fs");
	Shader queryShader("object_query.vs", "object_query.fs");
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	ourShader.setMat4("projection", projection);
	ourShader.setMat4("view", view);
	queryShader.setMat4("projection", projection);
	queryShader.setMat4("view", view);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	ourShader.setMat4("model", model);
	queryShader.setMat4("model", model);

	Model ourModel("../First_Step_For_Task/removeHiddenFace.fbx");
	ourModel.Draw(ourShader);

	FbxString lFilePath("../First_Step_For_Task/removeHiddenFace.fbx");
	FbxManager* lManager = NULL;
	FbxScene* lScene = NULL;
	InitializeSdkObjects(lManager, lScene);
	bool lResult = LoadScene(lManager, lScene, lFilePath.Buffer());
	if (lResult)
	{
		FbxNode* rootNode = lScene->GetRootNode();
		processNode(rootNode, queryShader);
	}
	else {
		FBXSDK_printf("Call to LoadScene() failed.\n");
	}

	const char* lFilename = "removeHiddenFace1.fbx";
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

	glfwTerminate();
	return 0;
}