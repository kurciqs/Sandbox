cmake_minimum_required(VERSION 3.2)
project(stb)

# stb lib
add_library(stb STATIC
	src/stb.cpp
	include/stb_image.h)