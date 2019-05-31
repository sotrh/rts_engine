#ifndef RTS_TEXTURE_H
#define RTS_TEXTURE_H

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace engine
{
    class Texture {
    public:
        Texture(const unsigned char* data, const uint width, const uint height, const bool hasAlpha) : width(width), height(height) {
            glGenTextures(1, &texture);
            glBindTexture(1, texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            if (hasAlpha) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            }

            glGenerateMipmap(GL_TEXTURE_2D);
        }

        void use() {
            glBindTexture(GL_TEXTURE_2D, texture);
        }

        void cleanup() {
            if (texture != 0) {
                glDeleteTextures(1, &texture);
                texture = 0;
            }
            width = height = 0;
        }
        
        uint texture = 0, width = 0, height = 0;
    };


    class TextureManager {
    public:
        void load(std::string filename) {
            if (textureMap.find(filename) != textureMap.end()) {
                return;
            }

            int width, height, numChannels;
            unsigned char* data = stbi_load(filename.c_str(), &width, &height, &numChannels, 0);

            if (data) {
                textureMap.insert(std::make_pair(filename, Texture(data, width, height, numChannels > 3)));
            } else {
                std::cerr << "Unable to load texture '" << filename << "'" << std::endl;
            }

            stbi_image_free(data);
        }

        Texture* get(std::string id) {
            auto iterator = textureMap.find(id);
            if (iterator == textureMap.end()) {
                return nullptr;
            } else {
                return &iterator->second;
            }
        }

        void cleanup() {
            for (auto pair : textureMap) {
                pair.second.cleanup();
            }
        }
    private:
        std::map<std::string, Texture> textureMap;
    };
} // namespace engine


#endif//RTS_TEXTURE_H