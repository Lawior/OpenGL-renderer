//those macros need to be shared between state and the main program, I am starting to regret my choices
//including nuklear in different files is a real pain in the ass
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
//The following defines taken from example program as idk what to set them to anyway
#define MAX_VERTEX_MEMORY 512 * 1024 //512 kb 
#define MAX_ELEMENT_MEMORY 128 * 1024
