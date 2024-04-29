#pragma once
#include <iostream>
#include <stb_image.h>
#include <ostream>
#include <GL/glew.h>

class Texture
{
public:
	unsigned int ID;

	Texture() : ID(0) {}

	unsigned int Create(const std::string& strTexturePath)
	{
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		unsigned char* data = stbi_load(strTexturePath.c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glGenTextures(1, &ID);
			glBindTexture(GL_TEXTURE_2D, ID);

			GLenum format = GL_RGB;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			// set the texture wrapping parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			// set texture filtering parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, 0);  // Deblocarea texturii dupã configurare
			stbi_image_free(data);
		}
		else {
			std::cout << "Failed to load texture: " << strTexturePath << std::endl;
		}
		stbi_image_free(data);

		return ID;
	}
};

