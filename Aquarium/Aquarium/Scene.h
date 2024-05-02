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
	// renders the 3D scene
// --------------------
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
		//glm::mat4 model;


		//// Cube 0
		//setModelTransform(model, glm::vec3(0.0f, 1.75f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(2.0f, 1.0f, 1.0f));
		//shader.SetMat4("model", model);
		//cube.renderCube();

		glm::mat4 model;
		setModelTransform(model, glm::vec3(0.0f, 1.75f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(2.0f, 1.0f, 1.0f));
		shader.SetMat4("model", model);
		//set alpha value to 0.5
		shader.SetFloat("transparency", 0.5f);

		cube.renderCube(texture);
	}

	//void renderScene(const Shader& shader,std::vector<unsigned int> textures)
	//{
	//	// floor
	//	glm::mat4 model;
	//	shader.SetMat4("model", model);
	//	Floor floor;
	//	floor.renderFloor(textures[0]);
	//	CubeObj cube;


 //       // Cube 3
 //       setModelTransform(model, glm::vec3(-3.0f, 0.5f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), 30.0f, glm::vec3(0.6f));
 //       shader.SetMat4("model", model);
 //       cube.renderCube(textures[1]);

 //       // Cube 4
 //       setModelTransform(model, glm::vec3(-2.0f, 0.5f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 30.0f, glm::vec3(0.5f));
 //       shader.SetMat4("model", model);
 //       cube.renderCube(textures[1]);
 //   }

    void renderScene(Shader& shader, std::vector<unsigned int> textures)
    {
        // floor
        glm::mat4 model;
        shader.SetMat4("model", model);
        Floor floor;
        floor.renderFloor(textures[1]);
        //CubeObj cube;

       // renderCubes(const_cast<Shader&>(shader), cube, textures[0]);


    }
	
};

