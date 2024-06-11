SHADER_STRINGIFY(
#if defined(VERT_SHADER)
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 tex_coords;

    uniform mat4 orthoProjection;
    uniform mat4 view_matrix;

    uniform float zoom;
    out float o_zoom;

    out vec2 o_tex_coords;
    uniform sampler2D tex;
    uniform float time;
    out float o_time;


    void main()
    {
        o_zoom = zoom;

        gl_Position = orthoProjection * view_matrix * vec4(aPos,1.0);
        o_tex_coords = tex_coords;
        o_time = time;
    };

#undef VERT_SHADER
#elif defined(FRAG_SHADER)

    out vec4 FragColor;
    in vec2 o_tex_coords;
    in float o_time;

    in float o_zoom;

    uniform sampler2D tex;
    void main()
    {
        if (o_zoom < 0.8f)
        {
            FragColor   = texture(tex, o_tex_coords);
            float timer = clamp(sin(o_time/8), 0.5, 1);
            FragColor  *= vec4(timer, 0.5, timer, 1);
        }
        else
        {
            FragColor   = texture(tex, o_tex_coords);
            float timer = clamp(sin(o_time/1), 0.5, 1);
            FragColor  *= vec4(timer, 0.5, timer, 1);
        }
    }

#undef FRAG_SHADER
#endif
)
