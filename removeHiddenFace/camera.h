#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

//相机属性默认值
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


//基于欧拉角的相机系统
class Camera
{
public:
	//相机属性
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	//欧拉角
	float Yaw;
	float Pitch;

	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	//构造函数：由向量创建
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	//构造函数：由标量创建
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	//LookAt矩阵
	glm::mat4 GetViewMatrix(glm::vec3 targetPosition);

	//处理键盘输出
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	//处理鼠标移动
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	//处理鼠标缩放
	void ProcessMouseScroll(float yoffset);

private:
	//更新相机矩阵
	void updateCameraVectors();
};
#endif