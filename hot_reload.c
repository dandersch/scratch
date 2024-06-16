#include "common.h"


void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar* message, const void* userParam)
{
    fprintf(stderr, "%s\n", message);
}

#define SHADER_STRINGIFY(x) "#version 330\n" #x
#define VERT_SHADER
const char* vertex_shader_source =
      #include "shader.glsl"
    ;
#define FRAG_SHADER
const char* fragment_shader_source =
      #include "shader.glsl"
    ;

#define VERT_SHADER
const char* vertex_shader2_source =
      #include "shader2.glsl"
    ;
#define FRAG_SHADER
const char* fragment_shader2_source =
      #include "shader2.glsl"
    ;

#define SHADERS(X) \
    X(0, vertex_shader_source, fragment_shader_source ) \
    X(1, vertex_shader2_source, fragment_shader2_source )

/* contains all state of the program */
typedef struct state_t {
    GLuint VAO, VBO;

    #define COUNT_SHADERS(a,...) ___##a,
    enum {
        SHADERS(COUNT_SHADERS)
        SHADER_COUNT
    };
    GLuint shaders[SHADER_COUNT];
    int current_shader;

    float cam_x, cam_y;
    float zoom;
    int mouse_wheel;
} state_t;

// loading a texture based of a char array
typedef struct char_to_color_t { char character; unsigned char color[4]; } char_to_color_t;
char_to_color_t char_to_color_map[] = {
    {'#', {255,255,255,0}},
    {'^', {255,255,0,0}},
    {'@', {255,0,0,0}},
    {' ', {0,0,0,255}},
    {'.', {0,0,0,0}}
};
const int width    = 16;
const int height   = 16;
const char bitmap[]  = {
    "                "
    "  ^ ^ ^  ^ ^ ^  "
    "  ^ ^ ^  ^ ^ ^  "
    "  ^^^^^^^^^^^^  "
    "  ^@^@^@@^@^@^  "
    "  ^^^^^^^^^^^^  "
    " ############## "
    " #....####....# "
    " #....####....# "
    " #..###..###..# "
    " #####....##### "
    "  ############  "
    "  #.########.#  "
    "  #.#.#..#.#.#  "
    "    #.#..#.#    "
    "                "
};
const int tex_mode = GL_RGBA;
unsigned char texture[64][4];

typedef struct vertex_t
{
    float vert_x, vert_y, vert_z;
    float tex_x,  tex_y;
} vertex_t;
#define VERTEX_LAYOUT(X) \
   X(0, 3, vert_x)       \
   X(1, 2, tex_x)

const float x = 100.0f;
const float y = 100.0f; // position for rendered quad on screen
const float tex_size_x = (float) width * 8;
const float tex_size_y = (float) height * 8;
// vertices for a quad
const vertex_t vertices[] = {
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
};

/* helper function */
GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader Compilation failed: %s\n", infoLog);
        return 0;
    }
    return shader;
}

GLuint upload_uniforms(GLuint shader, float cam_pos_x, float cam_pos_y, float zoom) {
    GLuint success = 1;

    /* orthographic projection */
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // NOTE hardcoded

    float left   = 0.0f;
    float right  = (float) SCREEN_WIDTH;
    float bottom = 0.0f;
    float top    = (float) SCREEN_HEIGHT;
    float near   = -1.0f;
    float far    = 1.0f;

    //zoom = 0.8f;
    left *= zoom;
    right *= zoom;
    bottom *= zoom;
    top *= zoom;

    float orthoProjection[16] = {
        2.0f / (right - left),                                        0.0f,                         0.0f, 0.0f,
        0.0f,                                        2.0f / (top - bottom),                         0.0f, 0.0f,
        0.0f,                                                         0.0f,         -2.0f / (far - near), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f
    };

    /* upload uniforms */
    glUseProgram(shader); // NOTE: must be called before uploading uniform
    int orthoLocation = glGetUniformLocation(shader, "orthoProjection");
    if (orthoLocation == -1) { success = 0; printf("Uniform orthoProjection not found\n"); }
    glUniformMatrix4fv(orthoLocation, 1, GL_FALSE, orthoProjection);

    float view_matrix[16] = {
        1.0f,  0.0f, 0.0f, 0.0f,
        0.0f,  1.0f, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
      -cam_pos_x,-cam_pos_y, 0.0f, 1.0f,
    };
    int view_matrix_uniform_location = glGetUniformLocation(shader, "view_matrix");
    if (view_matrix_uniform_location == -1) { success = 0; printf("Uniform view_matrix not found\n"); }
    glUniformMatrix4fv(view_matrix_uniform_location, 1, GL_FALSE, view_matrix);

    int zoom_loc = glGetUniformLocation(shader, "zoom");
    if (zoom_loc == -1) { success = 0; printf("Uniform zoom not found\n"); }
    glUniform1f(zoom_loc, zoom);

    static float time = 0; /* use for lack of a dt for now */
    glUniform1f(glGetUniformLocation(shader, "time"), time++);

    return success;
}

GLuint create_shader_program(const char* vertex_src, const char* frag_src) {
    GLuint vertex_shader   = compile_shader(GL_VERTEX_SHADER, vertex_src);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, frag_src);

    if (!vertex_shader)   { return 0; }
    if (!fragment_shader) { return 0; }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

EXPORT void render(state_t* state) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.2f, 0.1f, 0.2f);

    glUseProgram(state->shaders[state->current_shader]);
    upload_uniforms(state->shaders[state->current_shader], state->cam_x, state->cam_y, state->zoom);

    glBindVertexArray(state->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

int init_renderer(state_t* state) {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_callback, NULL);

    /* generate and bind vertex array object and vertex buffer object */
    glGenVertexArrays(1, &state->VAO);
    glGenBuffers(1, &state->VBO);

    glBindVertexArray(state->VAO);

    /* upload vertices */
    glBindBuffer(GL_ARRAY_BUFFER, state->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    #define FILL_ATTRIB_POINTER(index, count, member) \
        glVertexAttribPointer(index, count, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, member)); \
        glEnableVertexAttribArray(index);
    VERTEX_LAYOUT(FILL_ATTRIB_POINTER)

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return 0;
}

void generate_texture_and_upload() {
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

    /* generate texture */
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    // how to sample the texture when its larger or smaller
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /* upload texture */
    glTexImage2D(GL_TEXTURE_2D, 0, tex_mode, width, height, 0, tex_mode, GL_UNSIGNED_BYTE, texture);

    /* enable blending for transparency */
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_ALPHA);
}

EXPORT int on_reload(state_t* state) {

    #define CREATE_SHADER(idx,vert_src,frag_src) state->shaders[idx] = create_shader_program(vert_src, frag_src);
    SHADERS(CREATE_SHADER)

    generate_texture_and_upload();
    return 1;
}

EXPORT int on_load(state_t** state) {
    (*state) = malloc(sizeof(state_t));

    init_renderer(*state);
    on_reload(*state);

    (*state)->current_shader = 0;

    return 1;
}

EXPORT void update(state_t* state, float pos_x, float pos_y, int mouse_wheel) {
    state->cam_x = pos_x;
    state->cam_y = pos_y;
    state->mouse_wheel = mouse_wheel;
    state->zoom -= (mouse_wheel * 0.1);
}

EXPORT void mouse_click(state_t* state, int x, int y) {
    state->current_shader++;
    if (state->current_shader >= SHADER_COUNT) { state->current_shader = 0; }

    printf("Using shader: %i\n", state->current_shader);
    printf("Cam: %f %f\n", state->cam_x, state->cam_y);
    printf("Mouse at: %i %i\n", x, y);
    printf("Wheel at: %i\n", state->mouse_wheel);
}
