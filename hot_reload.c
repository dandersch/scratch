#include "common.h"

#define SHADER_STRINGIFY(x) "#version 330\n" #x

const char* vertex_shader_source =
      #define VERT_SHADER
      #include "shader.glsl"
      #undef VERT_SHADER
    ;

const char* fragment_shader_source =
      #define FRAG_SHADER
      #include "shader.glsl"
      #undef FRAG_SHADER
    ;

// loading a texture based of a char array
typedef struct char_to_color_t { char character; unsigned char color[4]; } char_to_color_t;
char_to_color_t char_to_color_map[] = {
    {'#', {255,255,255,0}},
    {' ', {0,0,0,255}},
    {'.', {0,0,0,0}}
};
const int width    = 8;
const int height   = 8;
const char bitmap[]  = {
    " ###### "
    "#..##..#"
    "#..##..#"
    "#.####.#"
    "###..###"
    " ###### "
    " #.##.# "
    " #.##.# "
};
const int tex_mode = GL_RGBA;
unsigned char texture[64][4];

typedef struct vertex_t
{
    float vert_x, vert_y, vert_z;
    float tex_x,  tex_y;
} vertex_t;

const float x = 200.0f, y = 200.0f; // position for rendered quad on screen
const float tex_size_x = 8.0f, tex_size_y = 8.0f;
// vertices for a quad
vertex_t vertices[] = {
#if 0
      // bottom right tri
      {
         -0.5f, -0.5f, 0.0f, // bottom left
          0.0f,  0.0f,       // uv
      },
      {
          0.5f, -0.5f, 0.0f, // bottom right
          1.0f,  0.0f,       // uv
      },
      {
         0.5f,  0.5f, 0.0f, // top right
         1.0f,  1.0f,       // uv
      },

     // upper left tri
     {
        0.5f,  0.5f, 0.0f, // top right
        1.0f,  1.0f,       // uv
     },
     {
       -0.5f,  0.5f, 0.0f, // top left
        0.0f,  1.0f,       // uv
     },
     {
       -0.5f, -0.5f, 0.0f, // bottom left
        0.0f,  0.0f,       // uv
     }
#else
     // bottom right tri
     {
         x,        y,        0.0f, // bottom left
         0.0f,     0.0f,           // uv
     },
     {
         x + tex_size_x, y,        0.0f, // bottom right
         1.0f,           0.0f,           // uv
     },
     {
         x + tex_size_x, y + tex_size_y, 0.0f, // top right
         1.0f,          1.0f,                  // uv
     },

     // upper left tri
     {
        x + tex_size_x, y + tex_size_y, 0.0f, // top right
        1.0f,           1.0f,                 // uv
     },
     {
        x, y + tex_size_y, 0.0f,  // top left
        0.0f,  1.0f,              // uv
     },
     {
        x,        y,        0.0f, // bottom left
        0.0f,     0.0f,           // uv
     }
#endif
};

// helper function
GLuint compile_shader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
        return 0;
    }
    return shader; // NOTE unused
}

GLuint create_shader_program(state_t* state)
{
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

    if (!vertex_shader)   { return 0; }
    if (!fragment_shader) { return 0; }

    state->shader_program = glCreateProgram();
    glAttachShader(state->shader_program, vertex_shader);
    glAttachShader(state->shader_program, fragment_shader);
    glLinkProgram(state->shader_program);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(state->shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(state->shader_program, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    // orthographic projection
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // NOTE hardcoded

    float left   = 0.0f;
    float right  = (float) SCREEN_WIDTH;
    float bottom = 0.0f;
    float top    = (float) SCREEN_HEIGHT;
    float near   = -1.0f;
    float far    = 1.0f;

    float orthoProjection[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f / (far - near), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f
    };

    // upload uniform
    glUseProgram(state->shader_program); // NOTE: must be called before uploading uniform
    int orthoLocation = glGetUniformLocation(state->shader_program, "orthoProjection");
    if (orthoLocation == -1) { printf("Uniform not found\n"); }
    glUniformMatrix4fv(orthoLocation, 1, GL_FALSE, orthoProjection);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return state->shader_program;
}

int render(state_t* state)
{
    // Render
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.2f, 0.1f, 0.2f);
    glUseProgram(state->shader_program);
    glBindVertexArray(state->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    return 0;
}

int hello(state_t* state)
{
    printf("Hello from DLL\n");
    return 0;
}

int init_renderer(state_t* state)
{
    // Generate and bind vertex array object and vertex buffer object
    glGenVertexArrays(1, &state->VAO);
    glGenBuffers(1, &state->VBO);

    glBindVertexArray(state->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, state->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, vert_x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, tex_x));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // fill texture buffer based on char array
    for (int i = 0; i < (width * height); i++)
    {
        /* load texture in up-side down (opengl'ism) */
        int row = i / width;
        int col = i % width;
        int new_row = (height - 1) - row;
        int i_inverted = (new_row * width + col);

        int found_a_map = 0;
        for (int j = 0; j < sizeof(char_to_color_map); j++)
        {
            if (char_to_color_map[j].character == bitmap[i])
            {
                found_a_map = 1;
                texture[i_inverted][0] = char_to_color_map[j].color[0];
                texture[i_inverted][1] = char_to_color_map[j].color[1];
                texture[i_inverted][2] = char_to_color_map[j].color[2];
                texture[i_inverted][3] = char_to_color_map[j].color[3];
            }
        }

        if (!found_a_map) { printf("No mapping to a color found for character %c\n", bitmap[i]); }
    }

    // upload texture
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    // how to sample the texture when its larger or smaller
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // mipmapping stuff, all turned off
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, GL_LINEAR);

    // wrap/clamp uv coords
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, tex_mode, width, height, 0, tex_mode, GL_UNSIGNED_BYTE, texture);

    // enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_ALPHA);

    return 0;
}
