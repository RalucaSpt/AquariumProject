#include <iostream>
#include "Camera.h"

#include <gtc/matrix_transform.hpp>
#include <GL/glew.h>


Camera::Camera(const int width, const int height, const glm::vec3& position)
	: startPosition(position)
{

	Set(width, height, position);
	mode = CameraMode::FreeLook;
}

glm::vec3 Camera::GetPosition()
{
	return position;
}

void Camera::Set(const int width, const int height, const glm::vec3& position)
{
	this->isPerspective = true;
	this->yaw = YAW;
	this->pitch = PITCH;

	this->FoVy = FOV;
	this->width = width;
	this->height = height;
	this->zNear = zNEAR;
	this->zFar = zFAR;

	this->worldUp = glm::vec3(0, 1, 0);
	this->position = position;

	lastX = width / 2.0f;
	lastY = height / 2.0f;
	bFirstMouseMove = true;

	UpdateCameraVectors();
}

void Camera::Reset(const int width, const int height)
{
	Set(width, height, startPosition);
}

void Camera::Reshape(int windowWidth, int windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	// define the viewport transformation
	glViewport(0, 0, windowWidth, windowHeight);

	// Adaugare log pentru debug
}

const glm::mat4 Camera::GetViewMatrix() const
{
	// Returns the View Matrix
	return glm::lookAt(position, position + forward, up);
}

const glm::mat4 Camera::GetProjectionMatrix() const
{
	glm::mat4 Proj = glm::mat4(1);
	if (isPerspective) {
		float aspectRatio = ((float)(width)) / height;
		Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
	}
	else {
		float scaleFactor = 2000.f;
		Proj = glm::ortho<float>(
			-width / scaleFactor, width / scaleFactor,
			-height / scaleFactor, height / scaleFactor, -zFar, zFar);
	}
	return Proj;
}

void Camera::ProcessKeyboard(ECameraMovementType direction, double deltaTime)
{
	float velocity = (float)(cameraSpeedFactor * deltaTime);
	if (mode == FreeLook)
	{
		switch (direction) {
		case ECameraMovementType::FORWARD:
			position += forward * velocity;
			break;
		case ECameraMovementType::BACKWARD:
			position -= forward * velocity;
			break;
		case ECameraMovementType::LEFT:
			position -= right * velocity;
			break;
		case ECameraMovementType::RIGHT:
			position += right * velocity;
			break;
		case ECameraMovementType::UP:
			position += up * velocity;
			break;
		case ECameraMovementType::DOWN:
			position -= up * velocity;
			break;
		}
	}
	else
	{
		switch (direction) {
		case ECameraMovementType::FORWARD:
			position += forward * velocity;
			break;
		case ECameraMovementType::BACKWARD:
			position -= forward * velocity;
			break;
		case ECameraMovementType::LEFT:
			position -= right * velocity;
			rotation.z += 0.6f;
			break;
		case ECameraMovementType::RIGHT:
			position += right * velocity;
			rotation.z -= 0.6f;
			break;
		case ECameraMovementType::UP:
			position += up * velocity;
			rotation.x += 0.6f;
			break;
		case ECameraMovementType::DOWN:
			position -= up * velocity;
			rotation.x -= 0.6f;
			break;
		}
	}
}

void Camera::MouseControl(float xPos, float yPos)
{
	if (bFirstMouseMove) {
		lastX = xPos;
		lastY = yPos;
		bFirstMouseMove = false;
	}

	float xChange = xPos - lastX;
	float yChange = lastY - yPos;
	lastX = xPos;
	lastY = yPos;

	if (fabs(xChange) <= 1e-6 && fabs(yChange) <= 1e-6) {
		return;
	}
	xChange *= mouseSensitivity;
	yChange *= mouseSensitivity;
	//if (mode == CameraMode::FreeLook) 
	ProcessMouseMovement(xChange, yChange);
}

void Camera::ProcessMouseScroll(float yOffset)
{
	if (FoVy >= 1.0f && FoVy <= 90.0f) {
		FoVy -= yOffset;
	}
	if (FoVy <= 1.0f)
		FoVy = 1.0f;
	if (FoVy >= 90.0f)
		FoVy = 90.0f;
}

void Camera::SetCameraMode(CameraMode mode)
{
	this->mode = mode;
}
CameraMode Camera::GetCameraMode()
{
	return this->mode;
}

void Camera::SetRotation(glm::vec3 rotation)
{
	this->rotation = rotation;
}

glm::vec3 Camera::GetRotation()
{
	return rotation;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch /*= true*/)
{
	yaw += xOffset;
	pitch += yOffset;

	if (constrainPitch) {
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
	// Calculate the new forward vector
	this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->forward.y = sin(glm::radians(pitch));
	this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->forward = glm::normalize(this->forward);
	// Also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, forward));
}

void Camera::SetPosition(const glm::vec3 vec)
{
	position = vec;
}

void Camera::SetYaw(float yaw)
{
	this->yaw = yaw;
	UpdateCameraVectors();
}

void Camera::SetPitch(float pitch)
{
	this->pitch = pitch;
}

void Camera::SetRoll(float x)
{
	this->roll = x;
}