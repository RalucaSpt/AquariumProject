#pragma once
#include <vec3.hpp>
#include <ext/matrix_transform.hpp>

#include "CubeObj.h"
#include "Floor.h"
#include "Model.h"
#include "Shader.h"

class Shader;

class Scene
{
	
public:
	void setModelTransform(glm::mat4& model,
		const glm::vec3& translateVector,
		const glm::vec3& rotateAxis,
		float rotateAngle,
		const glm::vec3& scaleVec)
	{
		model = glm::mat4();
		model = glm::translate(model, translateVector);
		model = glm::rotate(model, glm::radians(rotateAngle), rotateAxis);
		model = glm::scale(model, scaleVec);
	};

	void renderCubes(Shader& shader, CubeObj& cube, unsigned int texture)
	{

		glm::mat4 model;
		setModelTransform(model, glm::vec3(0.0f, 1.75f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(2.0f, 1.0f, 1.0f));
		shader.SetMat4("model", model);
		//set alpha value to 0.5
		shader.SetFloat("transparency", 0.5f);
		cube.renderCube(texture);
	}

    void renderScene(Shader& shader, std::vector<unsigned int> textures)
    {
        glm::mat4 model;
        shader.SetMat4("model", model);
        Floor floor;
        floor.renderFloor(textures[1]);
    }
	
};

