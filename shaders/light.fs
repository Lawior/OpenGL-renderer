#version 410 core
//Shader for the light source?
out vec4 color_final;

in vec2 TexCoord;

uniform vec3 object_color;
uniform vec3 light_color;
uniform sampler2D u_texture;

void main()
{
    color_final = texture(u_texture, TexCoord) * vec4( object_color,1);
}