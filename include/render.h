#ifndef RTS_RENDER_H
#define RTS_RENDER_H

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace engine {

    class SelectionBoxRenderer {
        public:
            void init() {
                cleanup();

                glGenVertexArrays(1, &_vao);
                glGenBuffers(1, &_vbo);
                glGenBuffers(1, &_ebo);

                glBindVertexArray(_vao);

                // _vbo -> x, y 
                glBindBuffer(GL_ARRAY_BUFFER, _vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float) * NUM_VERTICES * NUM_FLOATS_PER_VERTEX, nullptr, GL_DYNAMIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, NUM_FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(float) * NUM_FLOATS_PER_VERTEX, nullptr);

                // _ebo
                ushort indices[] = {
                    0, 1, 2,
                    0, 2, 3,
                };
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

                uint vs, fs;
                int status;
                char infoLog[256];

                auto vsCode = R"(
                    #version 330 core
                    layout(location=0) in vec2 aPosition;
                    void main() {
                        gl_Position = vec4(aPosition, 0.0, 1.0);
                    }
                )";
                vs = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vs, 1, &vsCode, nullptr);
                glCompileShader(vs);

                glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
                if (status == GL_FALSE) {
                    glGetShaderInfoLog(vs, 256, nullptr, infoLog);
                    std::cerr << "Unable to compile vertex shader\n" << infoLog << std::endl;
                }

                auto fsCode = R"(
                    #version 330 core
                    out vec4 fColor;
                    uniform vec4 uColor;
                    void main() {
                        fColor = uColor;
                    }
                )";
                fs = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fs, 1, &fsCode, nullptr);
                glCompileShader(fs);

                glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
                if (status == GL_FALSE) {
                    glGetShaderInfoLog(fs, 256, nullptr, infoLog);
                    std::cerr << "Unable to compile fragment shader\n" << infoLog << std::endl;
                }

                _program = glCreateProgram();
                glAttachShader(_program, vs);
                glAttachShader(_program, fs);
                glLinkProgram(_program);

                glGetProgramiv(_program, GL_LINK_STATUS, &status);
                if (status == GL_FALSE) {
                    glGetProgramInfoLog(_program, 256, nullptr, infoLog);
                    std::cerr << "Unable to link program\n" << infoLog << std::endl;
                }

                glDeleteShader(vs);
                glDeleteShader(fs);
            }

            void cleanup() {
                if (_vbo != 0) {
                    glDeleteBuffers(1, &_vbo);
                }
                if (_ebo != 0) {
                    glDeleteBuffers(1, &_ebo);
                }
                if (_vao != 0) {
                    glDeleteVertexArrays(1, &_vao);
                }
                if (_program != 0) {
                    glDeleteProgram(_program);
                }
            }

            void update(float minX, float minY, float maxX, float maxY) {
                float vertices[] = {
                    minX, minY, // bottom left
                    maxX, minY, // bottom right
                    maxX, maxY, // top right
                    minX, maxY, // top left
                };
                glBindVertexArray(_vao);
                glBindBuffer(GL_ARRAY_BUFFER, _vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            }

            void use() {
                glBindVertexArray(_vao);
                glUseProgram(_program);
            }

            void render(glm::vec4 color) {
                glBindVertexArray(_vao);
                glUseProgram(_program);
                glUniform4f(glGetUniformLocation(_program, "uColor"), color.r, color.g, color.b, color.a);
                glDrawElements(GL_TRIANGLES, NUM_INDICES, GL_UNSIGNED_SHORT, nullptr);
            }

        private:
            uint _vao, _vbo, _ebo, _program;
            static const uint NUM_VERTICES = 4, NUM_INDICES = 6, NUM_FLOATS_PER_VERTEX = 2;
    };

    class EntityRenderer {
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

                uniform vec3 uPosition;

                void main() {
                    gl_Position = vec4(aPosition + uPosition, 1.0);
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

                out vec4 fColor;

                uniform vec3 uColor;

                void main() {
                    fColor = vec4(uColor, 1.0);
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

        void render(glm::vec3& position, glm::vec3& color) {
            glUniform3f(glGetUniformLocation(_program, "uPosition"), position.x, position.y, position.z);
            glUniform3f(glGetUniformLocation(_program, "uColor"), color.x, color.y, color.z);
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