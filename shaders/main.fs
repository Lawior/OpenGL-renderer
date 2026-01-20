#version 410 core
    out vec4 color_final;

    //interestingly enough all the inputs in fragments shader get interpolated according to some algorithm
    //so essentialy new normals and positions for each pixel are calculated automatically
    in vec2 TexCoord;
    in vec3 normal;
    in vec3 fragment_pos;
    in vec3 light_pos_view;

    uniform vec3 object_color;
    uniform vec3 light_color;
    //uniform vec3 light_pos;
    uniform sampler2D u_texture;

    
    void main()
    {
        vec3 norm = normalize(normal); // normalizing normal because it may not be a normal due to interpolation
        float ambient_strength = 0.1f;
        float specular_strength = 0.4f;
        vec3 light_dir = normalize(light_pos_view - fragment_pos);

        vec3 ambient = ambient_strength * light_color;

        vec3 diffuse_light = max(dot(light_dir, norm), 0) * light_color;

        vec3 camera_pos = vec3(0,0,0); 
        vec3 view_dir = normalize(-fragment_pos);
        vec3 reflect_dir = reflect(-light_dir, norm);

        vec3 specular_light = pow(max(dot(view_dir, reflect_dir),0),32) * light_color * specular_strength;

        vec3 light_color_final = (diffuse_light + ambient + specular_light) * object_color;
        color_final = texture(u_texture, TexCoord) * vec4(light_color_final,1);
    }