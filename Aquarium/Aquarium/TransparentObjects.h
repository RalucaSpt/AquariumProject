#pragma once
#include "Camera.h"
#include "CubeObj.h"
#include "Shader.h"
#include "Scene.h"
class TransparentObjects
{
public:

	void Draw(::Shader& transparentShader, ::Camera* pCamera, unsigned texture, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, glm::
	          mat4 projection, glm::mat4 view, Scene scene)
	{

        transparentShader.Use();
        transparentShader.SetMat4("projection", projection);
        transparentShader.SetMat4("view", view);
        transparentShader.SetVec3("viewPos", pCamera->GetPosition());
        transparentShader.SetVec3("lightPos", lightPos);
        transparentShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        

        //make the cube object visible from all angles

        glDisable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        scene.renderCubes(const_cast<Shader&>(transparentShader), cube, texture);
	}

private:
 
	std::vector<unsigned int> texturePaths;
    CubeObj cube;
};

