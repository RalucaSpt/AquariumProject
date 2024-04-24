#pragma once
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>

#include "Shader.h"
#include "TextureStruct.h"
#include "Vertex.h"

#include <string>
#include <vector>

using namespace std;

class Mesh
{
public:
    // mesh Data
    unsigned int numVertices;
    std::shared_ptr <Vertex> vertices;

    unsigned int numIndexes;
    std::shared_ptr <unsigned int> indices;
    vector<TextureStruct>      textures;
    unsigned int VAO;

    Mesh(const vector<Vertex>& vertices, const vector<unsigned int>& indices, const vector<TextureStruct>& textures);
    Mesh(unsigned int numVertices, std::shared_ptr <Vertex> vertices, unsigned int numIndexes, std::shared_ptr <unsigned int> indices, const vector<TextureStruct>& textures);
    void Draw(Shader& shader);
private:
    // render data 
    unsigned int VBO, EBO;
    void setupMesh();
};

