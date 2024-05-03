#pragma once
#include <GL/glew.h>

class CubeObj
{

	// renderCube() renders a 1x1 3D cube in NDC.
	// -------------------------------------------------
public:
    void renderCube(unsigned int texture)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        // initialize (if necessary)
        if (cubeVAO == 0)
        {
            // Adjusted winding order to render both sides of each face
            float vertices[] =
            {
                // back face
                -12.5f, -12.5f, -12.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                12.5f,   12.5f, -12.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                12.5f,  -12.5f, -12.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
                12.5f,   12.5f, -12.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -12.5f, -12.5f, -12.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -12.5f,  12.5f, -12.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -12.5f, -12.5f,  12.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                12.5f,  -12.5f,  12.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                12.5f,   12.5f,  12.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                12.5f,   12.5f,  12.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -12.5f,  12.5f,  12.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -12.5f, -12.5f,  12.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -12.5f,  12.5f,  12.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // top-right
                -12.5f,  12.5f, -12.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                -12.5f, -12.5f, -12.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // bottom-left
                -12.5f, -12.5f, -12.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // bottom-left
                -12.5f, -12.5f,  12.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                -12.5f,  12.5f,  12.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                12.5f,  12.5f,  12.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // top-left
                12.5f, -12.5f, -12.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                12.5f,  12.5f, -12.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
                12.5f, -12.5f, -12.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // bottom-right
                12.5f,  12.5f,  12.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // top-left
                12.5f, -12.5f,  12.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f  // bottom-left   
            };


            glGenVertexArrays(1, &cubeVAO);
            glGenBuffers(1, &cubeVBO);
            // fill buffer
            glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            // link vertex attributes
            glBindVertexArray(cubeVAO);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        // render Cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

	

	GLuint GetTextureId();

private:
	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;
	GLuint textureId;
};

