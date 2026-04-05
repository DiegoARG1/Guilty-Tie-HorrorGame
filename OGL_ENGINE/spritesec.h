#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <glad/glad.h>
#include <stb/stb_image.h> // ¡La librería mágica que descubrimos en tu QuadTexture!

using namespace std;

class spritesec {
public:
    vector<unsigned int> texturasAnimacion;
    int frameActual;
    float temporizador;
    float velocidad;

    // CONSTRUCTOR: Carga las texturas leyendo los archivos directamente (Paso 3)
    spritesec(vector<string> rutas, float vel) {
        frameActual = 0;
        temporizador = 0.0f;
        velocidad = vel;

        for (int i = 0; i < rutas.size(); i++) {
            unsigned int textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            int width, height, nrChannels;
            // Carga la imagen usando la misma función que tu QuadTexture.h
            unsigned char* data = stbi_load(rutas[i].c_str(), &width, &height, &nrChannels, 0);

            if (data) {
                GLenum format = GL_RGB;
                if (nrChannels == 1) format = GL_RED;
                else if (nrChannels == 4) format = GL_RGBA;

                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                cout << "Error al cargar textura animada: " << rutas[i] << endl;
            }
            stbi_image_free(data);

            texturasAnimacion.push_back(textureID);
        }
    }

    // ACTUALIZAR: Indexar texturas con el timer de la aplicación (Paso 4)
    void actualizar(float deltaTime) {
        temporizador += deltaTime;
        if (temporizador >= velocidad) {
            frameActual++;
            if (frameActual >= texturasAnimacion.size()) {
                frameActual = 0;
            }
            temporizador = 0.0f;
        }
    }

    unsigned int getTexturaActual() {
        return texturasAnimacion[frameActual];
    }
};