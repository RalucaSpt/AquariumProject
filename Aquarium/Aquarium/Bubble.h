#pragma once
#include <fwd.hpp>
#include "Model.h"

class Bubble
{
public:
	Bubble();
	~Bubble();

	void Draw(const glm::mat4& view, const glm::mat4& projection);
	//void ResetModel() { m_model = glm::mat4(1.0f); }
private:
	//glm::mat4 m_model;

};

