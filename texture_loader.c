#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/gl.h>

GLuint loadTexture(const char* filename) {
    GLuint texture_ID;
    int width, height, channels;
    // Carrega a imagem usando stb_image
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 0);
    if(data) {
        glGenTextures(1, &texture_ID);
        glBindTexture(GL_TEXTURE_2D, texture_ID);

        // Configura a textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Envia a textura para a GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);

        // Libera a memória da imagem
        stbi_image_free(data);
        printf("Textura carregada: %s (ID: %u)\n", filename, texture_ID);
    }
    else {
        printf("Falha ao carregar a textura: %s\n", filename);
        texture_ID = 0;
    }

    return texture_ID;
}


