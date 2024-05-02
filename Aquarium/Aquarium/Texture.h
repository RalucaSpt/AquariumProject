#pragma once
#include <iostream>
#include <stb_image.h>
#include <ostream>
#include <GL/glew.h>

class Texture
{
public:
	unsigned int m_textureID;
	std::string m_textureType;
	std::string m_texturePath;

	Texture() = default;
	//copy constructor
	Texture(const Texture& texture)
	{
		m_textureID = texture.m_textureID;
		m_textureType = texture.m_textureType;
		m_texturePath = texture.m_texturePath;
	}

	//overload assignment operator
	Texture& operator=(const Texture& texture)
	{
		if (this != &texture)
		{
			m_textureID = texture.m_textureID;
			m_textureType = texture.m_textureType;
			m_texturePath = texture.m_texturePath;
		}
		return *this;
	}

	Texture(const std::string& strTexturePath, float alpha)
	{
		CreateTexture(strTexturePath, alpha);
	}

	unsigned int CreateTexture(const std::string& strTexturePath, float alpha);
	unsigned int GetTextureID();
	void SetTextureID(unsigned int textureID);
	std::string GetTextureType();
	void SetTextureType(std::string textureType);
	std::string GetTexturePath();
	void SetTexturePath(std::string texturePath);
};


