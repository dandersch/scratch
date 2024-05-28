SHADER_STRINGIFY(
#if defined(VERT_SHADER)

    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 tex_coords;
    uniform mat4 orthoProjection;
    out vec2 o_tex_coords;
    uniform sampler2D tex;

    void main()
    {
        vec2 screen_size = vec2(600,600); // NOTE hardcoded for now; pass in as uniform
        //vec2 tex_size = textureSize(tex, 0);
        //vec2 position_on_screen = screen_size * (vec2(aPos) + vec2(1,1));
        gl_Position = orthoProjection * vec4(aPos, 1.0);
        o_tex_coords = tex_coords;
    };

#elif defined(FRAG_SHADER)

    out vec4 FragColor;
    in vec2 o_tex_coords;
    uniform sampler2D tex;
    void main()
    {
        FragColor = texture(tex, o_tex_coords);
    }

#endif
)
