#ifndef RTS_ENGINE_H
#define RTS_ENGINE_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <queue>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <entityx/entityx.h>
#include <map>

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

    struct Selection {
        Selection(uint cursor, float minX, float minY, float maxX, float maxY)
            : cursor(cursor), minX(minX), minY(minY), maxX(maxX), maxY(maxY) {}
        uint cursor;
        float minX, minY, maxX, maxY;
    };

    struct SelectionChangedEvent {
        SelectionChangedEvent(Selection selection) : selection(selection) {}
        Selection selection;
    };

    struct SelectionEndedEvent {
        SelectionEndedEvent(Selection selection) : selection(selection) {}
        Selection selection;
    };

    class SelectionSystem : public entityx::System<SelectionSystem>, public entityx::Receiver<SelectionSystem> {
    public:
        SelectionSystem(SelectionBoxRenderer& renderer) 
            : renderer(renderer), selection(0, 0, 0, 0, 0), selectionColor(0, 1, 1, 0.1f) {}

        void configure(entityx::EventManager& eventManager) {
            eventManager.subscribe<SelectionChangedEvent>(*this);
            eventManager.subscribe<SelectionEndedEvent>(*this);
        }

        void update(entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt) {
            if (isSelecting) {
                renderer.render(selectionColor);
                entities.each<Position>([this](entityx::Entity entity, Position& position) {
                    auto pos = position.value;
                    bool isSelected = false;

                    if (pos.x > selection.minX && pos.y > selection.minY && pos.x < selection.maxX && pos.y < selection.maxY) {
                        if (!entity.has_component<Selection>()) {
                            entity.assign_from_copy<Selection>(selection);
                        }
                        isSelected = true;
                    }

                    if (!isSelected && entity.has_component<Selection>()) {
                        entity.remove<Selection>();
                    }
                });
            }
        }

        void receive(const SelectionChangedEvent &event) {
            selection = event.selection;
            isSelecting = true;
            renderer.update(selection.minX, selection.minY, selection.maxX, selection.maxY);
        }

        void receive(const SelectionEndedEvent &event) {
            isSelecting = false;
        }

    private:
        Selection selection;
        bool isSelecting;
        glm::vec4 selectionColor;
        SelectionBoxRenderer& renderer;
    };


    struct Job {
        Job(float x = 0.0f, float y = 0.0f, float z = 0.0f) : target(x, y, z) {}
        explicit Job(glm::vec3& target) : target(target) {}
        glm::vec3 target;

        // GotoJob
        // ItemSpawner/ItemCache
        // CollectItemJob
        // DepositItemJob
        // Item
        // Change how velocity works
    };

    struct JobAddedEvent {
        JobAddedEvent(Job job) : job(job) {}
        Job job;
    };

    class JobSystem : public entityx::System<JobSystem>, public entityx::Receiver<JobSystem> {
    public:
        void configure(entityx::EventManager& eventManager) {
            eventManager.subscribe<JobAddedEvent>(*this);
        }

        void update(entityx::EntityManager& es, entityx::EventManager& events, entityx::TimeDelta dt) override {
            es.each<Position, Velocity, Job>([dt, this](entityx::Entity entity, Position& position, Velocity& velocity, Job& job) {
                auto direction = job.target - position.value;
                if (glm::length(direction) < 0.01f) {
                    entity.remove<Job>();
                    velocity.value.x = 0;
                    velocity.value.y = 0;
                    velocity.value.z = 0;
                } else {
                    auto speed = 0.2f;
                    velocity.value = glm::normalize(direction) * speed;
                }
            });

            if (jobQueue.size() > 0) {
                entityx::ComponentHandle<Position> position;
                entityx::ComponentHandle<Velocity> velocity;
                entityx::ComponentHandle<Selection> selection;

                bool jobTaken = false;
                for (entityx::Entity entity : es.entities_with_components(position, velocity, selection)) {
                    if (entity.has_component<Job>()) continue;

                    auto job = jobQueue.front().job;
                    entity.assign_from_copy<Job>(job);
                    jobTaken = true;
                }
                if (jobTaken) jobQueue.pop();

                for (entityx::Entity entity : es.entities_with_components(position, velocity)) {
                    if (jobQueue.size() <= 0) break;
                    if (entity.has_component<Job>()) continue;

                    auto job = jobQueue.front().job;
                    jobQueue.pop();
                    entity.assign_from_copy<Job>(job);
                }
            }
        }

        void receive(const JobAddedEvent& event) {
            jobQueue.push(event);
        }

    private:
        std::queue<JobAddedEvent> jobQueue;
    };


    class EntityRenderSystem : public entityx::System<EntityRenderSystem> {
    public:
        EntityRenderSystem(EntityRenderer& renderer) : renderer(renderer) {}
        void update(entityx::EntityManager& es, entityx::EventManager& events, entityx::TimeDelta dt) override {
            if (renderer.isInitialized()) {
                renderer.use();
                es.each<Position>([this](entityx::Entity entity, Position& position) {
                    auto pos = position.value;
                    glm::vec3 color(0.0f, 0.0f, 0.0f);

                    if (entity.has_component<Selection>()) {
                        color.g = color.b = 1.0f;
                    } 
                    if (entity.has_component<Job>()) {
                        color.r = 1.0f;
                    }

                    renderer.render(position.value, color);
                });
            }
        }
    private:
        EntityRenderer& renderer;
    };

    class World : public entityx::EntityX {
    public:
        World(EntityRenderer& renderer, SelectionBoxRenderer& selectionBoxRenderer) {
            systems.add<MovementSystem>();
            systems.add<JobSystem>();
            systems.add<EntityRenderSystem>(renderer);
            systems.add<SelectionSystem>(selectionBoxRenderer);
            systems.configure();

            for (uint u = 0; u < 10; u++) {
                entityx::Entity entity = entities.create();
                entity.assign<Position>((float) rand() / RAND_MAX - 0.5f, (float) rand() / RAND_MAX - 0.5f, 0.0f);
                entity.assign<Velocity>(0.0f, 0.0f, 0.0f);
            }
        }

        void update(entityx::TimeDelta dt) {
            systems.update<MovementSystem>(dt);
            systems.update<JobSystem>(dt);
            systems.update<SelectionSystem>(dt);
            systems.update<EntityRenderSystem>(dt);
        }

        void addTarget(glm::vec3 target) {
            events.emit<JobAddedEvent>(Job(target));
        }

        void select(Selection selection) {
            events.emit<SelectionChangedEvent>(selection);
        }

        void deselect(Selection selection) {
            events.emit<SelectionEndedEvent>(selection);
        }
    };
}

#endif