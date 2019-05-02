#ifndef RTS_ENGINE_H
#define RTS_ENGINE_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <entityx/entityx.h>

#include "render.h"

namespace engine {
    struct Position {
        Position(float x, float y, float z) : value(x, y, z) {}
        glm::vec3 value;
    };

    struct Velocity {
        Velocity(float x, float y, float z) : value(x, y, z) {}
        glm::vec3 value;
    };

    class MovementSystem : public entityx::System<MovementSystem> {
    public:
        void update(entityx::EntityManager& es, entityx::EventManager& events, entityx::TimeDelta dt) override {
            es.each<Position, Velocity>([dt](entityx::Entity entity, Position& position, Velocity& velocity) {
                position.value += velocity.value * static_cast<float>(dt);

                if (position.value.x > 1) {
                    position.value.x = 1;
                    velocity.value.x *= -1;
                } else if (position.value.x < -1) {
                    position.value.x = -1;
                    velocity.value.x *= -1;
                }

                if (position.value.y > 1) {
                    position.value.y = 1;
                    velocity.value.y *= -1;
                } else if (position.value.y < -1) {
                    position.value.y = -1;
                    velocity.value.y *= -1;
                }
            });
        }
    };

    class DebugRenderSystem : public entityx::System<DebugRenderSystem> {
    public:
        DebugRenderSystem(DebugRenderer& renderer) : renderer(renderer) {}
        void update(entityx::EntityManager& es, entityx::EventManager& events, entityx::TimeDelta dt) override {
            if (renderer.isInitialized()) {
                renderer.use();
                es.each<Position>([this](entityx::Entity entity, Position& position) {
                    renderer.render(position.value);
                });
            }
        }
    private:
        DebugRenderer& renderer;
    };

    class World : public entityx::EntityX {
    public:
        World(DebugRenderer& renderer) {
            systems.add<MovementSystem>();
            systems.add<DebugRenderSystem>(renderer);
            systems.configure();

            for (uint u = 0; u < 10; u++) {
                entityx::Entity entity = entities.create();
                entity.assign<Position>((float) rand() / RAND_MAX - 0.5f, (float) rand() / RAND_MAX - 0.5f, 0.0f);

                float angle = (float) rand() / RAND_MAX * 2.0f * M_PIf32;
                float speed = 0.2f;
                entity.assign<Velocity>((float) cos(angle) * speed, (float) sin(angle) * speed, 0.0f);
            }
        }

        void update(entityx::TimeDelta dt) {
            systems.update<MovementSystem>(dt);
            systems.update<DebugRenderSystem>(dt);
        }
    };
}

#endif