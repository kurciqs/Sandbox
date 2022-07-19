#ifndef SANDBOX_UTILS_H
#define SANDBOX_UTILS_H

#include <malloc.h>
#include <io.h>

#ifdef WIN32
#include <windows.h>
#endif

#define print_log(format, ...) _print_log(__FILE__, __LINE__, format, __VA_ARGS__)
#define print_error(format, ...) _print_error(__FILE__, __LINE__, format, __VA_ARGS__)
#define offsetofi(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))

template <typename T1, typename T2>
inline size_t offset_of(T1 T2::*member);

const char* load_file(const char* path);

void _print_error(const char* file, int line, const char* format, ...);
void _print_log(const char* file, int line, const char* format, ...);
void get_exe_path(wchar_t * path);
bool fix_working_directory_path();

#endif //SANDBOX_UTILS_H
