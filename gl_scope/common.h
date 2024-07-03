#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH  300
#define SCREEN_HEIGHT 300

/* define export declaration for .dll & .so files */
#if defined(_MSC_VER)
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

#endif // COMMON_H_
