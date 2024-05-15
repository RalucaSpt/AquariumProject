#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	Init(vertexPath, fragmentPath);
}

Shader::~Shader()
{
	glDeleteProgram(ID);
}