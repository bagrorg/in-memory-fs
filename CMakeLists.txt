cmake_minimum_required(VERSION 3.12)
project(imfs)

add_subdirectory(src/storage)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} storage)

add_subdirectory(tests)