#ifndef RTS_RENDER_H
#define RTS_RENDER_H

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace engine {
    class DebugRenderer {
    public:
        void init() {
            cleanup();

            float vertices[] = {
                -0.02f, -0.02f, 0.0f,
                0.02f, -0.02f, 0.0f,
                0.0f,  0.02f, 0.0f,
            };

            glGenVertexArrays(1, &_vao);
            glGenBuffers(1, &_vbo);
            glBindVertexArray(_vao);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

            unsigned int vs, fs;
            vs = glCreateShader(GL_VERTEX_SHADER);
            auto vsSource = R"(
                #version 330 core

                layout(location=0) in vec3 aPosition;

                uniform vec3 iPosition;

                out vec3 vPosition;

                void main() {
                    vPosition = aPosition + iPosition;
                    gl_Position = vec4(vPosition, 1.0);
                }
            )";
            glShaderSource(vs, 1, &vsSource, nullptr);
            glCompileShader(vs);

            int status;
            char infoLog[256];
            glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
            if (!status) {
                glGetShaderInfoLog(vs, 256, nullptr, infoLog);
                std::cerr << "Unable to compile vertex shader\n" << infoLog << std::endl;
            }

            fs = glCreateShader(GL_FRAGMENT_SHADER);
            auto fsSource = R"(
                #version 330 core

                in vec3 vPosition;
                out vec4 fColor;

                void main() {
                    fColor = vec4(vPosition + vec3(0.5), 1.0);
                }
            )";
            glShaderSource(fs, 1, &fsSource, nullptr);
            glCompileShader(fs);

            glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
            if (!status) {
                glGetShaderInfoLog(fs, 256, nullptr, infoLog);
                std::cerr << "Unable to compile fragment shader\n" << infoLog << std::endl;
            }

            _program = glCreateProgram();
            glAttachShader(_program, vs);
            glAttachShader(_program, fs);
            glLinkProgram(_program);

            glDeleteShader(vs);
            glDeleteShader(fs);

            glGetProgramiv(_program, GL_LINK_STATUS, &status);
            if (!status) {
                glGetProgramInfoLog(_program, 256, nullptr, infoLog);
                std::cerr << "Unable to link shader _program\n" << infoLog << std::endl;
            }

            _isInitialized = true;
        }

        void cleanup() {
            _isInitialized = false;
            if (_vbo) {
                glDeleteBuffers(1, &_vbo);
            }
            if (_vao) {
                glDeleteVertexArrays(1, &_vao);
            }
            if (_program) {
                glDeleteProgram(_program);
            }
            _vbo = _vao = _program = 0;
        }

        void use() {
            glUseProgram(_program);
            glBindVertexArray(_vao);
        }

        void render(glm::vec3& position) {
            glUniform3f(glGetUniformLocation(_program, "iPosition"), position.x, position.y, position.z);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        bool isInitialized() {
            return _isInitialized;
        }

    private:
        uint _vao, _vbo, _program;
        bool _isInitialized = false;
    };
}

#endif//RTS_RENDER_H