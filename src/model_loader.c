#include <assimp/cimport.h>      // C importer interface
#include <assimp/scene.h>        // Output data structures
#include <assimp/postprocess.h>
#include <cglm/cglm.h>

#include "3d_engine/graphic_types.h"
#include "3d_engine/renderer.h"
#include "3d_engine/resource_manager.h"

#ifdef ASSIMP_DOUBLE_PRECISION
    #error "Code is not made to support double precision"
#endif

static void assimp_mat4_to_cglm_mat4(mat4 to, struct aiMatrix4x4* from)
{
    to[0][0] = from->a1; to[1][0] = from->a2; to[2][0] = from->a3; to[3][0] = from->a4;
    to[0][1] = from->b1; to[1][1] = from->b2; to[2][1] = from->b3; to[3][1] = from->b4;
    to[0][2] = from->c1; to[1][2] = from->c2; to[2][2] = from->c3; to[3][2] = from->c4;
    to[0][3] = from->d1; to[1][3] = from->d2; to[2][3] = from->d3; to[3][3] = from->d4;
}

// returns non 0 on success
//loads model at origin point 0 0 0 and at scale 1 1 1 
int load_static_model(char* path,
    Material* material // passing the material for now, as we don't load it from the model yet
)
{

    unsigned mesh_indices[MESH_PER_MODEL];
    Mesh meshes[MESH_PER_MODEL];
    char* folder = "models/";

    // should never be exceeded
    char full_model_path[512];
    snprintf(full_model_path, sizeof(full_model_path), "%s%s", folder, path);

    //This is what needs to be called to specify where assimp log should go
    //stdin for now
    struct aiLogStream s = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
    aiAttachLogStream(&s);
    
    struct aiScene* scene = aiImportFile(full_model_path, 
        aiProcess_Triangulate | //Change to triangles if the model is made of different primitives
        aiProcess_JoinIdenticalVertices | //seems self explanatory 
        aiProcess_FlipUVs | //file format specific
        aiProcess_GenNormals | // if no normals were specified, generate them NOTE: there is also aiProcess_GenSmoothNormals for non blocky models perhaps
        aiProcess_PreTransformVertices //this transforms all the meshes in to the model, losing the ability to use the same mesh many times in one model, and stuff like animation. 
    );
    if(scene == NULL  || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        printf("Assimp Error: %s\n", aiGetErrorString());
        return 0;
    }
    
    if(scene->mNumMeshes > MESH_PER_MODEL)
    {
        printf("Assimp Error: The model %s has %u exceeding the current max of %d\n", scene->mName, scene->mNumMeshes, MESH_PER_MODEL);
        return 0;
    }
    
    
    for (unsigned i = 0; i < scene->mNumMeshes; i++)
    {
        struct aiMesh* assimp_mesh = scene->mMeshes[i];
        
        struct aiMaterial* material = scene->mMaterials[assimp_mesh->mMaterialIndex];

        //Get the uvCoordinate coresponding to the diffuse texture, otherwise assume that it's at index 0
        unsigned uv_index = 0;
        //macro AI_MATKEY_UVWSRC_DIFFUSE(0) expands to the 3 arguments needed by the function
        if(aiGetMaterialInteger(material, AI_MATKEY_UVWSRC_DIFFUSE(0), &uv_index) != aiReturn_SUCCESS) uv_index = 0;

        if(assimp_mesh->mNormals == NULL)
        {
            printf("Assimp Error: No normals for static model loaded!");
            return 0;
        }
        if(assimp_mesh->mTextureCoords[0] == NULL)
        {
            printf("Assimp Error: Missing texture coordinates for static model");
            return 0;
        }

        unsigned vertices_stride = 3 + 2 + 3;
        unsigned texture_off = 3;
        unsigned normal_off = 3 + 2;

        //allocate the array of vertices + texture coord + normals (x,y,z, xt,yt xn,yn,zn)
        //NOTE: I may want to hold this in ram, otherwise it will be held in vram for the app's lifetime
        float* vertices = calloc(assimp_mesh->mNumVertices, vertices_stride * sizeof(float));
        if(vertices == NULL)
        {
            printf("Assimp Error: Calloc failed on vertices allocation");
            return 0;
        }

        for (unsigned j = 0; j < assimp_mesh->mNumVertices; j++)
        {

            memcpy(&vertices[j * vertices_stride], &assimp_mesh->mVertices[j], sizeof(float) * 3);
            memcpy(&vertices[j * vertices_stride + texture_off], &assimp_mesh->mTextureCoords[uv_index][j], sizeof(float) * 2); // copy the x, y of the texture coordinates ignore z
            memcpy(&vertices[j * vertices_stride + normal_off], &assimp_mesh->mNormals[j], sizeof(float) * 3);
        } 
        
        //assumed indices are triangles
        unsigned* indices = calloc(assimp_mesh->mNumFaces, sizeof(unsigned) * 3);
        unsigned indices_stride = 3;
        if(indices == NULL)
        {
            printf("Assimp Error: Calloc failed on indices allocation");
            return 0;
        }
        for(unsigned j = 0; j < assimp_mesh->mNumFaces; j++)
        {
            memcpy(&indices[j*indices_stride], assimp_mesh->mFaces[j].mIndices, sizeof(unsigned) * 3);
        }

        create_mesh(&meshes[i], 
            vertices, vertices_stride * assimp_mesh->mNumVertices * sizeof(float), 
            indices, indices_stride * assimp_mesh->mNumFaces * sizeof(unsigned)
        );
        mesh_indices[i] = rm_add_mesh(meshes[i]);
        
        free(indices);
        free(vertices);
    }
    
    model_create_named(path, scene->mNumMeshes, (vec3){0,0,0}, (vec3){0,0,0}, (vec3){1,1,1}, mesh_indices, material);

    aiReleaseImport(scene);
    aiDetachLogStream(&s);
    return 1;
}