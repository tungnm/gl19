#include "Texture.h"
#include <glad/glad.h>
#include <stb/stb_image.h>
#include <memory>
#include "Util.h"
#include <iostream>

unsigned int Texture::mCurrentTextureUnit = 0;
std::string Texture::TEXTURE_FOLDER_PATH = "textures\\";

void delete_stb_image_func(unsigned char* image)
{
    stbi_image_free(image);
}

bool Texture::LoadSingleImage(std::string filePath)
{
    std::string file = TEXTURE_FOLDER_PATH + filePath;

    if (!isFileExist(file))
    {
        return false;
    }

    glActiveTexture(GL_TEXTURE0 + mCurrentTextureUnit);
    // load texture

    glGenTextures(1, &mTextureHandle);
    glBindTexture(GL_TEXTURE_2D, mTextureHandle);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load image using stb
    int width, height, nrChannels;
    std::unique_ptr<unsigned char, void(*)(unsigned char*)> data(stbi_load(file.c_str(), &width, &height, &nrChannels, 0)
        , delete_stb_image_func);

    if (data.get())
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.get());
        glGenerateMipmap(GL_TEXTURE_2D);

        mTextureUnit = mCurrentTextureUnit;
        mCurrentTextureUnit++;
    }
    else
    {
        std::cout << "\n Failed to load texture";
    }
}

Texture::Texture()
{
    mTextureHandle = 0;
    mTextureUnit = 0;
}

Texture::~Texture()
{
    if (mTextureHandle > 0)
    {
        std::cout << "\n Delete texture: " << mTextureHandle;
        glDeleteTextures(1, &mTextureHandle);
    }
}

unsigned int Texture::GetTextureUnit()
{
    return mTextureUnit;
}