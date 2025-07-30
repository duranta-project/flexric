/*

Copyright (c) 2022 Evan Pezent & ImPlot Community

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <sstream>

struct Shader
{
    Shader() : ID(0) {}
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    ~Shader()
    {
        if (ID != 0)
            glDeleteProgram(ID);
    }

    bool LoadFromFile(const char *vertPath, const char *fragPath,
                      const char *geomPath = NULL)
    {
        std::ifstream vertFile, fragFile, geomFile;
        std::string vertCode, fragCode, geomCode;
        vertFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fragFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        geomFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            std::stringstream vertStream, fragStream, geomStream;

            vertFile.open(vertPath);
            vertStream << vertFile.rdbuf();
            vertFile.close();
            vertCode = vertStream.str();

            fragFile.open(fragPath);
            fragStream << fragFile.rdbuf();
            fragFile.close();
            fragCode = fragStream.str();

            if (geomPath != NULL)
            {
                geomFile.open(geomPath);
                geomStream << geomFile.rdbuf();
                geomFile.close();
                geomCode = geomStream.str();
            }
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "Failed to read shader files!" << std::endl;
            return false;
        }
        return LoadFromString(vertCode.c_str(), fragCode.c_str(),
                              geomPath == NULL ? NULL : geomCode.c_str());
    }

    bool LoadFromString(const char *vertSrc, const char *fragSrc,
                        const char *geomSrc = NULL)
    {
        int success;
        char infoLog[512];
        // compile vertex shader
        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vertSrc, NULL);
        glCompileShader(vertShader);
        glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
            std::cout << "Failed to compile vertex shader!\n"
                      << infoLog << std::endl;
            return false;
        }
        // compiler fragment shader
        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &fragSrc, NULL);
        glCompileShader(fragShader);
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
            std::cout << "Failed to compile fragment shader!\n"
                      << infoLog << std::endl;
            return false;
        }
        GLuint geomShader;
        if (geomSrc != NULL)
        {
            geomShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geomShader, 1, &geomSrc, NULL);
            glCompileShader(geomShader);
            glGetShaderiv(geomShader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(geomShader, 512, NULL, infoLog);
                std::cout << "Failed to compile geometry shader!\n"
                          << infoLog << std::endl;
                return false;
            }
        }
        // compile program
        ID = glCreateProgram();
        glAttachShader(ID, vertShader);
        if (geomSrc != NULL)
            glAttachShader(ID, geomShader);
        glAttachShader(ID, fragShader);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "Failed to link shader program!\n"
                      << infoLog << std::endl;
            return false;
        }
        // clean up
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        if (geomSrc != NULL)
            glDeleteShader(geomShader);
        return true;
    }

    void SetFloat(const char *name, float v) const
    {
        glUniform1f(glGetUniformLocation(ID, name), v);
    }

    void SetFloat2(const char *name, const ImVec2 &v) const
    {
        glUniform2f(glGetUniformLocation(ID, name), v.x, v.y);
    }

    void SetFloat4(const char *name, const ImVec4 &v) const
    {
        glUniform4f(glGetUniformLocation(ID, name), v.x, v.y, v.z, v.w);
    }

    void SetMat4(const char *name, const float *v) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, v);
    }

    void Use() { glUseProgram(ID); }

    GLuint ID = 0;
};
