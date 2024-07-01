#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// RESOURCES
// https://www.devever.net/~hl/incbin
// https://github.com/graphitemaster/incbin
// https://mort.coffee/home/fast-cpp-embeds/
// https://github.com/mortie/strliteral
// https://thephd.dev/finally-embed-in-c23#and-in-c-you-can-make-it-constexpr-which-means-you-can-check-man
// https://sentido-labs.com/en/library/cedro/202106171400/use-embed-c23-today.html

/*
** FIRST OPTION: convert binary files to hex-formated text files
** - adds xxd (or convert, bin2h, ...) as a build dependency...
** - OR requires writing custom program that can be included with the source code
** - requires modification to the build script if you want to keep binary files up-to-date
** - binary files can be included with their original name (if you want)
** - larger filesizes
** - you get the buffer as a real array (sizeof(buffer) works)
** - can be slooow
*/
unsigned char buffer[] = {
    #include "door.png"
};

/*
** SECOND OPTION: .o variant
** - Build dependency: using ld or objcopy (Windows: bin2coff, bin2obj)
** - (memory will be always const unless you memcpy)
** - you don't get a real array of the data, just a pointer and its size (i.e. you shouldn't call sizeof(buffer))
** - you can specify different types (not just char) if you want
** - you don't have access to the data or the size at compile-time (only after linking)
** - you have to "include" it by writing MY_INCLUDE(file, ext) instead of #include "file.ext"
** - smaller filesizes compared to first option (textfile vs binary object fiel)
** - requires (more complex) modifications to the build script than the first option
** - no additional build dependencies (linker was needed anyway)
** - probably faster compile times than the first option (xxd vs linker + no need to parse all those hex values)
*/
//extern const unsigned char _binary_filename_ext_start[];
//extern const unsigned char _binary_filename_ext_end[];
//extern const unsigned char _binary_filename_ext_size; // NOTE access with (size_t)&_size.
#define BINARY_INCLUDE(file, ext)                               \
  extern const unsigned char _binary_##file##_##ext##_start[];  \
  extern const unsigned char _binary_##file##_##ext##_end[]

//extern const unsigned char new_name[] asm("_binary_door_png_start");
//extern const unsigned char png_size asm("_binary_door_png_size");

#define BINARY_ADDRESS(file, ext) _binary_##file##_##ext##_start
#define BINARY_SIZE(file, ext)    _binary_##file##_##ext##_end - _binary_##file##_##ext##_start

#define BINARY_BUFFER(buffer_name, size_name, file, ext)          \
    const unsigned char* buffer_name = BINARY_ADDRESS(file, ext); \
    unsigned long        size_name   = BINARY_SIZE(file, ext)

BINARY_INCLUDE(door, png);

/*
** THIRD OPTION: inline assembly using .incbin
** - not very cross-platform: ~.incbin~ is a GNU-specific asm directive
** - buffer and size need to be ~extern~, meaning we again don't have access at compile-time
*/
#define BINARY_ASM_INCLUDE(filename, buffername)     \
    __asm__(".section .rodata\n"                     \
         ".global " #buffername "\n"                 \
         ".type   " #buffername ", @object\n"        \
         ".align  4\n"                               \
     #buffername":\n"                                \
         ".incbin " #filename "\n"                   \
     #buffername"_end:\n"                            \
         ".global "#buffername"_size\n"              \
         ".type   "#buffername"_size, @object\n"     \
         ".align  4\n"                               \
     #buffername"_size:\n"                           \
         ".int   "#buffername"_end - "#buffername"\n"\
    );                                               \
    extern const unsigned char buffername [];        \
    extern const unsigned char* buffername##_end;    \
    extern int buffername##_size

BINARY_ASM_INCLUDE("res/door.png", door_asm);

/*
** FOURTH OPTION: use library
** - crossplatform (with some restrictions in case of MSVC)
** - can use second option without requiring modifications to the build system
*/
// NOTE:
// The tool https://github.com/graphitemaster/incbin actually uses all of these approaches
// (First option in case of MSVC, since it doesn't support inline assembly)
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE // data instead of Data
#include "incbin.h"

INCBIN(door, "res/door.png"); // defines door_data door_end door_size

/*
** FIFTH OPTION: The #embed directive introduced by C23
** - current compilers do not seem to implement this feature yet
** - Usage-wise, this is similar to the first approach using tools like ~xxd~.
**   However, since this will have compiler support, it will presumably be much
**   faster.
** - Crossplatform, as long as the compiler supports C23
*/
// static const unsigned char embed_door[] = {
//     #embed "res/door.png"
// };

int main()
{
    BINARY_BUFFER(bin_buf, bin_buf_size, door, png);

    /* printf("%zu\n", (size_t) &png_size); */
    /* printf("%lu\n", bin_buf_size); */

    int width, height, nrChannels;

    unsigned char* data1 = stbi_load("res/door.png", &width, &height, &nrChannels, 0);
    if (!data1) { printf("Couldn't load data1\n"); }

    unsigned char* data2 = stbi_load_from_memory(buffer, sizeof(buffer), &width, &height, &nrChannels, 0);
    if (!data2) { printf("Couldn't load data2\n"); }

    unsigned char* data3 = stbi_load_from_memory(bin_buf, bin_buf_size, &width, &height, &nrChannels, 0);
    if (!data3) { printf("Couldn't load data3\n"); }

    unsigned char* data4 = stbi_load_from_memory(door_asm, door_asm_size, &width, &height, &nrChannels, 0);
    if (!data4) { printf("Couldn't load data4\n"); }

    unsigned char* data5 = stbi_load_from_memory(door_data, door_size, &width, &height, &nrChannels, 0);
    if (!data5) { printf("Couldn't load data4\n"); }

    if (memcmp(data1, data2, width * height * nrChannels) == 0) { printf("buffers are the same\n");     }
    else                                                        { printf("buffers are NOT the same\n"); }
    if (memcmp(data1, data3, width * height * nrChannels) == 0) { printf("buffers are the same\n");     }
    else                                                        { printf("buffers are NOT the same\n"); }
    if (memcmp(data1, data4, width * height * nrChannels) == 0) { printf("buffers are the same\n");     }
    else                                                        { printf("buffers are NOT the same\n"); }
    if (memcmp(data1, data5, width * height * nrChannels) == 0) { printf("buffers are the same\n");     }
    else                                                        { printf("buffers are NOT the same\n"); }
}
