SHADER_STRINGIFY(
    out vec4 FragColor;
    in vec2 o_tex_coords;
    in float o_tex_index;
    in vec4 o_color;
    uniform sampler2D u_tex_units[16];
    uniform float u_zoom;
    void main()
    {
        /* SUBPIXEL FILTERING **************************************************************/
        vec2 size = vec2(textureSize(u_tex_units[0], 0));
        switch(int(o_tex_index)) // TODO can we do this w/o a switch?
        {
            case  0:                                       ; break;
            case  1: size = textureSize(u_tex_units[ 1], 0); break;
            case  2: size = textureSize(u_tex_units[ 2], 0); break;
            case  3: size = textureSize(u_tex_units[ 3], 0); break;
            case  4: size = textureSize(u_tex_units[ 4], 0); break;
            case  5: size = textureSize(u_tex_units[ 5], 0); break;
            case  6: size = textureSize(u_tex_units[ 6], 0); break;
            case  7: size = textureSize(u_tex_units[ 7], 0); break;
            case  8: size = textureSize(u_tex_units[ 8], 0); break;
            case  9: size = textureSize(u_tex_units[ 9], 0); break;
            case 10: size = textureSize(u_tex_units[10], 0); break;
            case 11: size = textureSize(u_tex_units[11], 0); break;
            case 12: size = textureSize(u_tex_units[12], 0); break;
            case 13: size = textureSize(u_tex_units[13], 0); break;
            case 14: size = textureSize(u_tex_units[14], 0); break;
            case 15: size = textureSize(u_tex_units[15], 0); break;
        }
        vec2 uv = o_tex_coords;
        uv = uv * size;
        vec2 duv = fwidth(uv);
        uv = floor(uv) + vec2(0.5) + clamp((fract(uv) - vec2(0.5) + duv)/duv, 0, 1);
        uv /= size;
        // TODO clamp to nearest multiple of 16 ?
        /***********************************************************************************/
        FragColor = o_color;
        switch(int(o_tex_index))
        {
            case  0:                                          ; break;
            case  1: FragColor *= texture(u_tex_units[ 1], uv); break;
            case  2: FragColor *= texture(u_tex_units[ 2], uv); break;
            case  3: FragColor *= texture(u_tex_units[ 3], uv); break;
            case  4: FragColor *= texture(u_tex_units[ 4], uv); break;
            case  5: FragColor *= texture(u_tex_units[ 5], uv); break;
            case  6: FragColor *= texture(u_tex_units[ 6], uv); break;
            case  7: FragColor *= texture(u_tex_units[ 7], uv); break;
            case  8: FragColor *= texture(u_tex_units[ 8], uv); break;
            case  9: FragColor *= texture(u_tex_units[ 9], uv); break;
            case 10: FragColor *= texture(u_tex_units[10], uv); break;
            case 11: FragColor *= texture(u_tex_units[11], uv); break;
            case 12: FragColor *= texture(u_tex_units[12], uv); break;
            case 13: FragColor *= texture(u_tex_units[13], uv); break;
            case 14: FragColor *= texture(u_tex_units[14], uv); break;
            case 15: FragColor *= texture(u_tex_units[15], uv); break;
        }
        FragColor = vec4(1.0,1.0,1.0,1.0);
    }
)
