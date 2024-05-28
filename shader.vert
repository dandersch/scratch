SHADER_STRINGIFY(
uniform mat4  u_camera;
//layout (location = 0) in vec2 pos;
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in float tex_index;
layout (location = 3) in vec4 color;
out vec2 o_tex_coords;
out float o_tex_index;
out vec4 o_color;
void main()
{
    //gl_Position  = u_camera * vec4(pos.x, pos.y, 1.0, 1.0); // NOTE -y seems to fix orientation
    o_tex_coords = tex_coords;
    o_tex_index  = tex_index;
    o_color      = color;
    gl_Position  = vec4(aPos, 1.0f);
};
)
