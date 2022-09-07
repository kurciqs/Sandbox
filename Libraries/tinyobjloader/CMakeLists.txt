cmake_minimum_required(VERSION 3.2)
project(tinyobjloader)

add_library(tinyobjloader STATIC
        src/tinyobjloader.cpp
        include/tinyobjloader.h)