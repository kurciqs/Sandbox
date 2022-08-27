cmake_minimum_required(VERSION 3.2)
project(tinyobjloader)

# stb lib
add_library(tinyobjloader STATIC
        src/tinyobjloader.cpp
        include/tinyobjloader.h mapbox/earcut.hpp)