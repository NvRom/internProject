#pragma once
#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	Vertex(double x, double y, double z) :Position(x, y, z) {};
	Vertex() {};
};

struct Texture {
	unsigned int id;
	string type;
	string path;
};

class Mesh {
public:
	//Mesh的数据结构
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	//构造函数
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices);

	void Draw(Shader shader);
private:
	//用于渲染的VBO和EBO
	unsigned int VBO, EBO;

	//初始化mesh
	void setupMesh();
};
#endif