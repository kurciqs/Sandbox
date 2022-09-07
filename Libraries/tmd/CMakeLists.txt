cmake_minimum_required(VERSION 3.2)
project(tmd)

# tmd lib
add_library(tmd STATIC
        src/tmd.cpp
        include/tmd.h)