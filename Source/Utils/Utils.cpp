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

template <typename T1, typename T2>
size_t offset_of(T1 T2::*member) {
    static T2 obj;
    return size_t(&(obj.*member)) - size_t(&obj);
}