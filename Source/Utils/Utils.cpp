#include <cstdio>
#include "Utils.h"

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