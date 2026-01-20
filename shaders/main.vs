#version 410 core
   layout (location = 0) in vec3 aPos;
   layout (location = 1) in vec2 aTexCoord;
   layout (location = 2) in vec3 aNormal;

   uniform mat4 model;      // Local Space -> World Space
   uniform mat4 view;       // World Space -> View Space (Camera)
   uniform mat4 projection; // View Space  -> Clip Space (Screen)
   uniform mat3 normal_matrix; // Model matrix but cut (no need to move normal in x y z space) inversed and transposed
   uniform vec3 light_pos;
   uniform vec3 camera_pos;
   
   out vec2 TexCoord;
   out vec3 fragment_pos;
   out vec3 normal;
   out vec3 light_pos_view;

   void main()
   {
      gl_Position = projection * view * model * vec4(aPos, 1.0);
      TexCoord = aTexCoord; // pass this to the fragment shader 
      fragment_pos = vec3(view * model * vec4(aPos, 1.0));

      normal = mat3(view) * normal_matrix * aNormal;
      light_pos_view = vec3(view * vec4(light_pos, 1));
   }