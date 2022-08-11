#ifndef SANDBOX_UTILS_H
#define SANDBOX_UTILS_H

#include <cstdio>
#include "glm/glm.hpp"
#include <unistd.h>

#ifdef WIN32
#include <windows.h>
#endif

#define print_log(format, ...) _print_log(__FILE__, __LINE__, format, __VA_ARGS__)
#define print_error(format, ...) _print_error(__FILE__, __LINE__, format, __VA_ARGS__)

const char* load_file(const char* path);

void printMat3(glm::mat3 m);
void _print_error(const char* file, int line, const char* format, ...);
void _print_log(const char* file, int line, const char* format, ...);
void get_exe_path(wchar_t * path);
bool fix_working_directory_path();

#endif //SANDBOX_UTILS_H
