#include "Utils.h"

void printMat3(glm::mat3 m) {
    printf("\n%f %f %f\n%f %f %f\n%f %f %f\n\n", m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);
}

void _print_error(const char *file, int line, const char *format, ...) {
#ifdef WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
#endif

    printf("%s:%d [ERROR]: ", file, line);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

#ifdef WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif

    printf("\n");
}

void _print_log(const char *file, int line, const char *format, ...) {
    printf("%s:%d [LOG]: ", file, line);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

void getexepath(wchar_t* buf) {
#ifdef _WIN32
//    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
#else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
#endif
}