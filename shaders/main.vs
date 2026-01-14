#version 410 core
   layout (location = 0) in vec3 aPos;
   layout (location = 1) in vec2 aTexCoord;
   uniform mat4 model;      // Local Space -> World Space
   uniform mat4 view;       // World Space -> View Space (Camera)
   uniform mat4 projection; // View Space  -> Clip Space (Screen)

   out vec2 TexCoord;

   void main()
   {
      gl_Position = projection * view * model * vec4(aPos, 1.0);
      TexCoord = aTexCoord; // pass this to the fragment shader
   }