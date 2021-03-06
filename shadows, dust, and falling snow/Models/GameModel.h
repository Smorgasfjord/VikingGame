#ifndef __GAME_MODEL_H__
#define __GAME_MODEL_H__

//
// Lighthouse3D.com OpenGL 3.3 + GLSL 3.3 Sample
//
// Loading and displaying a Textured Model
//
// Uses:
//  Assimp lybrary for model loading
//    http://assimp.sourceforge.net/
//  Devil for image loading
//    http://openil.sourceforge.net/
// Uniform Blocks
//  Vertex Array Objects
//
// Some parts of the code are strongly based on the Assimp
// SimpleTextureOpenGL sample that comes with the Assimp
// distribution, namely the code that relates to loading the images
// and the model.
//
// The code was updated and modified to be compatible with
// OpenGL 3.3 CORE version
//
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//
// If you do use it I would love to hear about it. Just post a comment
// at Lighthouse3D.com

// Have Fun :-)


#include <cmath>

// include DevIL for image loading
#include </home/nclarke/Desktop/deps/devil/include/IL/il.h>

// include GLEW to access OpenGL 3.3 functions
#ifdef __unix__
#endif

#ifdef __APPLE__
//#include <glew.h>
#endif

#ifdef _WIN32
#pragma comment(lib,"assimp.lib")
#pragma comment(lib,"devil.lib")
#pragma comment(lib,"glew32.lib")
#endif

// assimp include files. These three are usually needed.
#include </home/nclarke/Desktop/deps/assimp/Importer.hpp>   //OO version Header!
#include </home/nclarke/Desktop/deps/assimp/postprocess.h>
#include </home/nclarke/Desktop/deps/assimp/scene.h>


#include </home/nclarke/Desktop/deps/glm/include/glm/glm.hpp>
#include </home/nclarke/Desktop/deps/glm/include/glm/gtc/type_ptr.hpp>


using namespace std;

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <stdlib.h>
#include "../Utils/GLHandles.h"

// This is for a shader uniform block
struct MyMaterial{

   float diffuse[4];
   float ambient[4];
   float specular[4];
   float emissive[4];
   float shininess;
   int texCount;
};
typedef struct MyMaterial MatData;

// Information to render each assimp node
struct MyMesh{

   GLuint vao;
   GLuint vbo;
   GLuint tbo;
   GLuint nbo;
   GLuint ibo;
   GLuint texIndex;
   GLuint uniformBlockIndex;
   MatData mat;
   int numFaces;
};
typedef struct MyMesh MeshBufferData;


// Uniform Bindings Points
///GLuint matricesUniLoc = 1, materialUniLoc = 2;

// The sampler uniform for textured models
// we are assuming a single texture so this will
//always be texture unit 0
/////GLuint texUnit = 0;

// Uniform Buffer for Matrices
// this buffer will contain 3 matrices: projection, view and model
// each matrix is a float array with 16 components
#define MatricesUniBufferSize sizeof(float) * 16 * 4
#define ProjMatrixOffset 0
#define ViewMatrixOffset sizeof(float) * 16
#define ModelMatrixOffset sizeof(float) * 16 * 2
#define NormMatrixOffset sizeof(float) * 16 * 3
#define MatrixSize sizeof(float) * 16

// the global Assimp scene object
//const aiScene* scene = NULL;

//-----------------------------------------------------------------
// Print for OpenGL errors
//
// Returns 1 if an OpenGL error occurred, 0 otherwise.
//

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line);

// ----------------------------------------------------
// MATRIX STUFF
//

// Push and Pop for modelMatrix

void pushMatrix(); 
void popMatrix();

// ----------------------------------------------------------------------------

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

void get_bounding_box_for_node (const aiNode* nd, aiVector3D* min, aiVector3D* max);

void get_bounding_box (aiVector3D* min, aiVector3D* max);
bool Import3DFromFile( const std::string& pFile, aiVector3D *min, aiVector3D *max, aiVector3D *center);

int LoadGLTextures();

//// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
//void Color4f(const aiColor4D *color)
//{
// glColor4f(color->r, color->g, color->b, color->a);
//}

void set_float4(float f[4], float a, float b, float c, float d);
void color4_to_float4(const aiColor4D *c, float f[4]);



class BufferContents {
   public:
      std::vector<glm::vec3> verts;
      std::vector<glm::vec3> norms;
      std::vector<glm::vec2> texes;
      std::vector<glm::vec3> faces;
      unsigned int numVerts;
      unsigned int numFaces;

      BufferContents()
      {
      }

      BufferContents(unsigned int nv, unsigned int nf):
         numVerts(nv),
         numFaces(nf)
      {
         verts.reserve(nv);
         norms.reserve(nv);
         texes.reserve(nv);
         faces.reserve(nf);
      }
};

class ModelMesh {
   public:
      MeshBufferData buffDat;
      unsigned int mIdx;

      ModelMesh()
      {
      }

      ModelMesh(MeshBufferData bd, unsigned int m) :
         mIdx(m) 
      {
         buffDat = bd;
      }
};

class ModelNode {
   public:
      std::vector<ModelMesh> meshes;
      std::vector<ModelNode> children;
      std::string name;
      glm::mat4 transform;

      ModelNode()
      {
      }

      ModelNode(std::string n, glm::mat4 t) :
         name(n),
         transform(t)
      {
      }

};

class GameModel {
   public:
      ModelNode rootNode;
      std::string fname;
      glm::vec3 min;
      glm::vec3 max;
      std::vector<MeshBufferData> meshData;
      std::vector<BufferContents> contents;

      GameModel()
      {
      }

      GameModel(std::string fn) :
         fname(fn)
      {
      }

      void updateBounds();
      void genVAOsAndUniformBuffer(const aiScene *sc, GLHandles handle); 
};

GameModel genSimpleModel(GameModel *mod);
ModelNode genModel(const aiScene *sc, std::vector<MeshBufferData> meshData);
GameModel loadModel(std::string fName, GLHandles handle);

#endif
