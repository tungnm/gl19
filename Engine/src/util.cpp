#include <string>
#include <fstream>
#include <iostream>

void printOpenGLVersion()
{

}

bool isFileExist(std::string fileName)
{
    std::ifstream ifs(fileName);
    if (ifs.fail())
    {
        std::cerr << "File not found: " << fileName << std::endl;
        return false;
    }
}