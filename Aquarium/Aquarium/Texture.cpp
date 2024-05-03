#include "Texture.h"
//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//#endif
unsigned Texture::CreateTexture(const std::string& strTexturePath, float alpha)
{
	unsigned int textureId = -1;

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(false); // tell stb_image.h to flip loaded texture's on the y-axis.


	unsigned char* data = stbi_load(strTexturePath.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha); // Ensure loading with alpha channel
	if (data) {
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // Use GL_RGBA for alpha channel

		// Adjust transparency by modifying alpha channel
		for (int i = 0; i < width * height * 4; i += 4) {
			data[i + 3] = static_cast<unsigned char>(alpha * 255); // Update alpha channel
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "Failed to load texture: " << strTexturePath << std::endl;
	}
	stbi_image_free(data);
	m_textureID = textureId;
	m_texturePath = strTexturePath;
	m_textureType = "texture_diffuse";

	return textureId;
}

unsigned Texture::GetTextureID()
{
	return m_textureID;
}

void Texture::SetTextureID(unsigned textureID)
{
	m_textureID = textureID;
}

std::string Texture::GetTextureType()
{
	return m_textureType;
}

void Texture::SetTextureType(std::string textureType)
{
	m_textureType = textureType;
}

std::string Texture::GetTexturePath()
{
	return m_texturePath;
}

void Texture::SetTexturePath(std::string texturePath)
{
	m_texturePath = texturePath;
}
