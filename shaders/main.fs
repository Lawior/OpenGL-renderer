#version 410 core
    out vec4 color_final;

    in vec2 TexCoord;

    uniform vec4 color;
    uniform sampler2D texture;
    
    void main()
    {
        color_final = texture2D(texture, TexCoord) * color; 
    }