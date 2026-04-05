#ifndef Agua_H
#define Agua_H

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <stb/stb_image.h>
#include <engine/shader_m.h>

using std::cerr;
using std::endl;
using std::string;
using std::vector;

class AguaPlano
{
    Shader* shader;

    unsigned int VAO, VBO, EBO;
    unsigned int cubemapTexture;
    unsigned int loadTextures()
    {
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        string path = string("textures/agua.jpg");
        int width, height, nrChannels;
       
        string file = path;
        unsigned char* data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);


        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
           
           /* cerr << file << endl;*/
           /* GLenum format = 1;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;*/

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
                
        return texture;
    }

public:

    AguaPlano(const float size = 1, const string agua = "1")
    {
        shader = new Shader("shaders/figures.vs", "shaders/figures.fs");
        GLfloat agua_vertices[] = {
            size, 0, -size,
            size, 0, size,
            -size, 0, -size,
            -size, 0, size};

        unsigned int agua_indices[] = {
            0, 1, 3,
            0, 3, 2};

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        cubemapTexture = loadTextures();
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(agua_vertices), agua_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(agua_indices), agua_indices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    };
    AguaPlano()
    {

    }
    ~AguaPlano()
    {

    }
  

    void Release()
    {
        delete shader;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
    void draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
    {
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", model);
       

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubemapTexture);
        glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);
        
    }
    Shader* getShader()
    {
        return this->shader;
    }
    void setShader(Shader* shader)
    {
        this->shader = shader;
    }
};

#endif