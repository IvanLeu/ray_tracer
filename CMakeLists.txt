cmake_minimum_required(VERSION 3.26)

project(RayTracer)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED)

add_subdirectory("src")