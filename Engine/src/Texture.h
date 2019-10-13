#pragma once
#include <string>

/*
 * This class represents a texture. It is responsible for
 * 1. loading texture into GPU
 * 2. keeping track of texture handle
 * 3. delete texture from GPU memory when the texture object itself is delete
 */
class Texture
{
private:
    static unsigned int mCurrentTextureUnit;
    static std::string TEXTURE_FOLDER_PATH;
    unsigned int mTextureHandle;
    unsigned int mTextureUnit;

public:
    bool LoadSingleImage(std::string filePath);
    unsigned int GetTextureUnit();
    Texture();
    ~Texture();
};