SHADER_STRINGIFY(
#if defined(VERT_SHADER)

    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 tex_coords;
    out vec2 o_tex_coords;
    void main()
    {
        gl_Position  = vec4(aPos, 1.0f);
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
