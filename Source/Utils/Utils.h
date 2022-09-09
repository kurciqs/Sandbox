#ifndef SANDBOX_UTILS_H
#define SANDBOX_UTILS_H

#include <iostream>
#include <cstdarg>
#include "glm/glm.hpp"
#include <unistd.h>

#ifdef WIN32
#include <windows.h>
#include <filesystem>

#endif

#define RANDOM_COLOR glm::vec3(rand() % 255, rand() % 255, rand() % 255) / 255.0f

#define print_log(format, ...) _print_log(__FILE__, __LINE__, format, __VA_ARGS__)
#define print_error(format, ...) _print_error(__FILE__, __LINE__, format, __VA_ARGS__)

void printMat3(glm::mat3 m);
void printVec3(glm::vec3 v);
void _print_error(const char* file, int line, const char* format, ...);
void _print_log(const char* file, int line, const char* format, ...);
float InvSqrt(float x);
glm::vec3 veryFastNormalize(glm::vec3 v);

void getexepath(wchar_t* buf);

#endif //SANDBOX_UTILS_H
