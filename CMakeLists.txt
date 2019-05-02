cmake_minimum_required(VERSION 3.2)

project(rts_engine VERSION 0.1.0)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw-3.3)

add_subdirectory(libs/glad)

add_subdirectory(libs/entityx)

add_subdirectory(libs/glm)

include_directories(include)
add_executable(game src/main.cpp)
target_link_libraries(game PRIVATE glfw)
target_link_libraries(game PRIVATE glfw)
target_link_libraries(game PRIVATE glad)
target_link_libraries(game PRIVATE entityx)
target_link_libraries(game PRIVATE glm)