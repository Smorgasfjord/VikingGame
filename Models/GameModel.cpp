#define AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE 80.0
#include "GameModel.h"

#define PI 3.14157

// Uniform Buffer for Matrices
// this buffer will contain 3 matrices: projection, view and model
// each matrix is a float array with 16 components
GLuint matricesUniBuffer;

// Create an instance of the Importer class
Assimp::Importer importer;

// scale factor for the model to fit in the window
float scaleFactor;

// Vertex Attribute Locations
GLuint vertexLoc=0, normalLoc=1, texCoordLoc=2;
//
// the global Assimp scene object
const aiScene* scene = NULL;
BufferContents conts;

// images / texture
// map image filenames to textureIds
// pointer to texture Array
std::map<std::string, GLuint> textureIdMap;

// Frame counting and FPS computation
char s[32];

//-----------------------------------------------------------------
// Print for OpenGL errors
//
// Returns 1 if an OpenGL error occurred, 0 otherwise.
//

int printOglError(char *file, int line)
{

    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    if (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d\n",
              file, line);
        retCode = 1;
    }
    return retCode;
}


// ----------------------------------------------------------------------------

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

void get_bounding_box_for_node (const aiNode* nd,
   aiVector3D* min,
   aiVector3D* max)

{
   aiMatrix4x4 prev;
   unsigned int n = 0, t;

   for (; n < nd->mNumMeshes; ++n) {
      const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
      for (t = 0; t < mesh->mNumVertices; ++t) {

         aiVector3D tmp = mesh->mVertices[t];

         min->x = aisgl_min(min->x,tmp.x);
         min->y = aisgl_min(min->y,tmp.y);
         min->z = aisgl_min(min->z,tmp.z);

         max->x = aisgl_max(max->x,tmp.x);
         max->y = aisgl_max(max->y,tmp.y);
         max->z = aisgl_max(max->z,tmp.z);
      }
   }

   for (n = 0; n < nd->mNumChildren; ++n) {
      get_bounding_box_for_node(nd->mChildren[n],min,max);
   }
}


void get_bounding_box (aiVector3D* min, aiVector3D* max)
{

   min->x = min->y = min->z =  1e10f;
   max->x = max->y = max->z = -1e10f;
   get_bounding_box_for_node(scene->mRootNode,min,max);
}


bool Import3DFromFile( const std::string& pFile, aiVector3D *min, aiVector3D *max, aiVector3D *center)
{

   //check if file exists
   std::ifstream fin(pFile.c_str());
   if(!fin.fail()) {
      fin.close();
   }
   else{
      printf("Couldn't open file: %s\n", pFile.c_str());
      printf("%s\n", importer.GetErrorString());
      return false;
   }

   scene = importer.ReadFile( pFile, aiProcessPreset_TargetRealtime_Quality | aiProcess_GenSmoothNormals);

   // If the import failed, report it
   if( !scene)
   {
      printf("%s\n", importer.GetErrorString());
      return false;
   }
   // Now we can access the file's contents.
   printf("Import of scene %s succeeded.\n",pFile.c_str());

   aiVector3D scene_min, scene_max, scene_center;
   get_bounding_box(min, max);
   *center = *max - *min;
   float tmp;
   tmp = max->x-min->x;
   tmp = max->y - min->y > tmp?max->y - min->y:tmp;
   tmp = max->z - min->z > tmp?max->z - min->z:tmp;
   scaleFactor = 1.f / tmp;

   // We're done. Everything will be cleaned up by the importer destructor
   return true;
}

int LoadGLTextures(std::string fName)
{
   ILboolean success;

   /* initialization of DevIL */
   ilInit();

      /* create and fill array with DevIL texture ids */
   ILuint imageId;
   ilGenImages(1, &imageId);

   /* create and fill array with GL texture ids */
   GLuint textureId;;
   glGenTextures(1, &textureId); /* Texture name generation */

   //save IL image ID
   std::string filename = fName;  // get filename

   ilBindImage(imageId); /* Binding of DevIL image name */
   ilEnable(IL_ORIGIN_SET);
   ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
   success = ilLoadImage((ILstring)filename.c_str());

   if (success) {
      /* Convert image to RGBA */
      ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

      /* Create and load textures to OpenGL */
      glBindTexture(GL_TEXTURE_2D, textureId);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
         ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
         ilGetData());
   }
   else
      printf("Couldn't load Image: %s\n", filename.c_str());
   /* Because we have already copied image data into texture data
   we can release memory used by image. */
   ilDeleteImages(1, &imageId);

   //return success;
   return textureId;
}


int LoadGLTextures()
{
   ILboolean success;

   /* initialization of DevIL */
   ilInit();

   /* scan scene's materials for textures */
   for (unsigned int m=0; m<scene->mNumMaterials; ++m)
   {
      int texIndex = 0;
      aiString path; // filename

      aiReturn texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
      while (texFound == AI_SUCCESS) {
         //fill map with textures, OpenGL image ids set to 0
         textureIdMap[path.data] = 0;
         // more textures?
         texIndex++;
         texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
      }
   }

   int numTextures = (int)textureIdMap.size();

   /* create and fill array with DevIL texture ids */
   ILuint* imageIds = new ILuint[numTextures];
   ilGenImages(numTextures, imageIds);

   /* create and fill array with GL texture ids */
   GLuint* textureIds = new GLuint[numTextures];
   glGenTextures(numTextures, textureIds); /* Texture name generation */

   /* get iterator */
   std::map<std::string, GLuint>::iterator itr = textureIdMap.begin();
   int i=0;
   for (; itr != textureIdMap.end(); ++i, ++itr)
   {
      //save IL image ID
      std::string filename = (*itr).first;  // get filename
      (*itr).second = textureIds[i];     // save texture id for filename in map

      ilBindImage(imageIds[i]); /* Binding of DevIL image name */
      ilEnable(IL_ORIGIN_SET);
      ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
      success = ilLoadImage((ILstring)filename.c_str());

      if (success) {
         /* Convert image to RGBA */
         ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

         /* Create and load textures to OpenGL */
         glBindTexture(GL_TEXTURE_2D, textureIds[i]);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
            ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
            ilGetData());
      }
      else
         printf("Couldn't load Image: %s\n", filename.c_str());
   }
   /* Because we have already copied image data into texture data
   we can release memory used by image. */
   ilDeleteImages(numTextures, imageIds);

   //Cleanup
   delete [] imageIds;
   delete [] textureIds;

   //return success;
   return true;
}



//// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
//void Color4f(const aiColor4D *color)
//{
// glColor4f(color->r, color->g, color->b, color->a);
//}

void set_float4(float f[4], float a, float b, float c, float d)
{
   f[0] = a;
   f[1] = b;
   f[2] = c;
   f[3] = d;
}

void color4_to_float4(const aiColor4D *c, float f[4])
{
   f[0] = c->r;
   f[1] = c->g;
   f[2] = c->b;
   f[3] = c->a;
}

glm::vec3 eulerAngles(glm::quat q1) {
   glm::vec3 ret;
   float sqw = q1.w*q1.w;
   float sqx = q1.x*q1.x;
   float sqy = q1.y*q1.y;
   float sqz = q1.z*q1.z;
   float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
   float test = q1.x*q1.y + q1.z*q1.w;
   if (test > 0.499*unit) { // singularity at north pole
      ret.y = 2.0 * atan2(q1.x,q1.w);
      ret.x = M_PI/2.0;
      ret.z = 0;
      return ret;
   }
   if (test < -0.499*unit) { // singularity at south pole
      ret.y = -2.0 * atan2(q1.x,q1.w);
      ret.x = -M_PI/2.0;
      ret.z = 0;
      return ret;
   }
   ret.y = atan2(2.0*q1.y*q1.w-2.0*q1.x*q1.z , sqx - sqy - sqz + sqw);
   ret.x = asin(2.0*test/unit);
   ret.z = atan2(2.0*q1.x*q1.w-2.0*q1.y*q1.z , -sqx + sqy - sqz + sqw);
   return ret;
}

BufferContents findBounds(BufferContents & bc) {
   BufferContents newBc = BufferContents(8,12);
   glm::vec3 min = glm::vec3(10000.0f), max = glm::vec3(-10000.0f), norm;
   for (int i = 0; i < bc.numVerts; i++) {
      if (bc.verts[i].x > max.x) max.x = bc.verts[i].x;
      else if (bc.verts[i].x < min.x) min.x = bc.verts[i].x;
      if (bc.verts[i].y > max.y) max.y = bc.verts[i].y;
      else if (bc.verts[i].y < min.y) min.y = bc.verts[i].y;
      if (bc.verts[i].z > max.z) max.z = bc.verts[i].z;
      else if (bc.verts[i].z < min.z) min.z = bc.verts[i].z;
   }
   norm = glm::normalize(max - min);
   newBc.verts.push_back(glm::vec3(min.x,min.y,min.z)); //lbb
   newBc.verts.push_back(glm::vec3(min.x,min.y,max.z)); //lbf
   newBc.verts.push_back(glm::vec3(min.x,max.y,max.z)); //ltf
   newBc.verts.push_back(glm::vec3(min.x,max.y,min.z)); //ltb
   newBc.verts.push_back(glm::vec3(max.x,max.y,min.z)); //rtb
   newBc.verts.push_back(glm::vec3(max.x,max.y,max.z)); //rtf
   newBc.verts.push_back(glm::vec3(max.x,min.y,max.z)); //rbf
   newBc.verts.push_back(glm::vec3(max.x,min.y,min.z)); //rbb
   newBc.faces.push_back(glm::vec3(0,1,2.0f));//right
   newBc.faces.push_back(glm::vec3(2,3,0.0f));
   newBc.faces.push_back(glm::vec3(2,3,4.0f));//top
   newBc.faces.push_back(glm::vec3(4,5,2.0f));
   newBc.faces.push_back(glm::vec3(4,5,6.0f));//left
   newBc.faces.push_back(glm::vec3(6,7,4.0f));
   newBc.faces.push_back(glm::vec3(0,1,6.0f));//bottom
   newBc.faces.push_back(glm::vec3(6,7,0.0f));
   newBc.faces.push_back(glm::vec3(4,3,0.0f));//back
   newBc.faces.push_back(glm::vec3(0,7,4.0f));
   newBc.faces.push_back(glm::vec3(1,2,5.0f));//front
   newBc.faces.push_back(glm::vec3(5,6,1.0f));
   newBc.norms.push_back(glm::vec3(-norm.x,-norm.y,-norm.z));
   newBc.norms.push_back(glm::vec3(-norm.x,-norm.y,norm.z));
   newBc.norms.push_back(glm::vec3(-norm.x,norm.y,norm.z));
   newBc.norms.push_back(glm::vec3(-norm.x,norm.y,-norm.z));
   newBc.norms.push_back(glm::vec3(norm.x,norm.y,-norm.z));
   newBc.norms.push_back(glm::vec3(norm.x,norm.y,norm.z));
   newBc.norms.push_back(glm::vec3(norm.x,-norm.y,norm.z));
   newBc.norms.push_back(glm::vec3(norm.x,-norm.y,-norm.z));
   return newBc;
}

ModelNode genSimpleModelNode(ModelNode *node, std::vector<MeshBufferData> meshData) {
   ModelNode nod;
   ModelMesh mesh;
   MeshBufferData mDat;
   unsigned int mIdx;

   //trans = glm::make_mat4x4(arr); //not working for some reason
   nod = ModelNode(node->name.c_str(), node->transform);
   for (int i = 0; i < node->meshes.size(); i++) {
      mIdx = node->meshes[i].mIdx;
      mDat = meshData[mIdx];
      mesh = ModelMesh(mDat,mIdx);
      nod.meshes.push_back(mesh);
   }
   for (int j = 0; j < node->children.size(); j++) {
      nod.children.push_back(genSimpleModelNode(&(node->children[j]), meshData));
   }
   return nod;
}

GameModel genSimpleModel(GameModel *mod) {
   GameModel newMod = GameModel(mod->fname.c_str());
   for (int i = 0; i < mod->contents.size(); i++) {
      newMod.contents.push_back(findBounds(mod->contents[i]));
   }

   newMod.rootNode = genSimpleModelNode(&(mod->rootNode), mod->meshData);
   return newMod;
}

ModelNode genModelNode(const aiNode *node, std::vector<MeshBufferData> & meshData, const aiAnimation *anim) {
   ModelNode nod;
   ModelMesh mesh;
   MeshBufferData mDat;
   float arr[16];
   unsigned int mIdx;
   glm::mat4 trans, mPos, mScale, mRot;
   glm::quat converter;
   KeyFrame frame;
   std::vector<glm::vec3> pos, scale, rot;

      arr[15] = node->mTransformation.a1;
      arr[14] = node->mTransformation.a2;
      arr[13] = node->mTransformation.a3;
      arr[12] = node->mTransformation.a4;
      arr[11] = node->mTransformation.b1;
      arr[10] = node->mTransformation.b2;
      arr[9] = node->mTransformation.b3;
      arr[8] = node->mTransformation.b4;
      arr[7] = node->mTransformation.c1;
      arr[6] = node->mTransformation.c2;
      arr[5] = node->mTransformation.c3;
      arr[4] = node->mTransformation.c4;
      arr[3] = node->mTransformation.d1;
      arr[2] = node->mTransformation.d2;
      arr[1] = node->mTransformation.d3;
      arr[0] = node->mTransformation.d4;
   printf("%1.3f %1.3f %1.3f %1.3f\n%1.3f %1.3f %1.3f %1.3f\n%1.3f %1.3f %1.3f %1.3f\n%1.3f %1.3f %1.3f %1.3f\n\n",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5],arr[6],arr[7],arr[8],arr[9],arr[10],arr[11],arr[12],arr[13],arr[14],arr[15]);
   //trans = glm::make_mat4x4(arr); //not working for some reason
   trans = glm::mat4(1.0f);
   nod = ModelNode(node->mName.C_Str(), trans);
   for (int i = 0; i < node->mNumMeshes; i++) {
      mIdx = node->mMeshes[i];
      mDat = meshData[mIdx];
      mesh = ModelMesh(mDat,mIdx);
      nod.meshes.push_back(mesh);
   }
   for (int j = 0; j < node->mNumChildren; j++) {
      nod.children.push_back(genModelNode(node->mChildren[j], meshData, anim));
   }
   if (anim != NULL) {
      for (int a = 0; a < anim->mNumChannels; a++) {
         if (anim->mChannels[a]->mNodeName == node->mName) {
            for (int k = 0; k < anim->mChannels[a]->mNumPositionKeys; k++) {
               pos.push_back(glm::vec3());
               pos[k].z = anim->mChannels[a]->mPositionKeys[k].mValue.x;
               pos[k].x = anim->mChannels[a]->mPositionKeys[k].mValue.y;
               pos[k].y = anim->mChannels[a]->mPositionKeys[k].mValue.z;
               printf("animation transation: %f,%f,%f\n", pos[k].x,pos[k].y,pos[k].z);
            }
            for (int k = 0; k < anim->mChannels[a]->mNumScalingKeys; k++) {
               scale.push_back(glm::vec3());
               scale[k].x = anim->mChannels[a]->mScalingKeys[k].mValue.x;
               scale[k].y = anim->mChannels[a]->mScalingKeys[k].mValue.y;
               scale[k].z = anim->mChannels[a]->mScalingKeys[k].mValue.z;
               printf("animation scaling: %f,%f,%f\n", scale[k].x,scale[k].y,scale[k].z);
            }
            for (int k = 0; k < anim->mChannels[a]->mNumRotationKeys; k++) {
               converter.x = anim->mChannels[a]->mRotationKeys[k].mValue.x;
               converter.y = anim->mChannels[a]->mRotationKeys[k].mValue.y;
               converter.z = anim->mChannels[a]->mRotationKeys[k].mValue.z;
               converter.w = anim->mChannels[a]->mRotationKeys[k].mValue.w;
               rot.push_back(eulerAngles(converter));
               printf("animation rotation: %f,%f,%f\n", rot[k].x*180.0f/(float)M_PI,rot[k].y*180.0f/(float)M_PI,rot[k].z*180.0f/(float)M_PI);
            }
            for (int m = 0; m < pos.size(); m++) {
               /*mPos = glm::translate(glm::mat4(1.0f), pos[m]);
               mScale = glm::scale(glm::mat4(1.0f), scale[m]);
               mRot = glm::rotate(glm::mat4(1.0f), rot[m].x*180.0f/(float)M_PI, glm::vec3(1.0f,0.0,0.0));
               mRot *= glm::rotate(glm::mat4(1.0f), rot[m].y*180.0f/(float)M_PI, glm::vec3(0.0f,1.0,0.0));
               mRot *= glm::rotate(glm::mat4(1.0f), rot[m].z*180.0f/(float)M_PI, glm::vec3(0.0f,0.0,1.0));
               trans = mPos * mRot * mScale;*/
               frame.t = pos[m];
               frame.s = scale[m];
               frame.r = rot[m];
               nod.keys.push_back(frame);
            }
         }
      }
   }
   return nod;
}

ModelNode genModel(const aiScene *sc, std::vector<MeshBufferData> & meshData) {
   ModelNode mod;

   if (sc->HasAnimations()) {
      mod = genModelNode(sc->mRootNode, meshData, sc->mAnimations[0]);
   }
   else {
      mod = genModelNode(sc->mRootNode, meshData, NULL);
   }

   return mod;
}

GameModel loadModel(std::string fName, GLHandles handle) {
   aiVector3D min, max, center;
   GameModel mod = GameModel(fName);
   if (!Import3DFromFile(fName,&min,&max,&center)) {
      return GameModel();
   }
   LoadGLTextures();
   mod.genVAOsAndUniformBuffer(scene, handle);
   mod.rootNode = genModel(scene, mod.meshData);
   return mod;
}

void GameModel::genVAOsAndUniformBuffer(const aiScene *sc, GLHandles handle) {

   static int wat = 0;
   MeshBufferData aMesh;
   VertexBoneData mBone;
   struct MyMaterial aMat;
   //GLsizei stride = (3 + 3 + 2) * sizeof(float);
   float *firstComp = 0;
   //GLuint buffer;
   contents = std::vector<BufferContents>();

   // For each mesh
   for (unsigned int n = 0; n < sc->mNumMeshes; ++n)
   {
      const aiMesh* mesh = sc->mMeshes[n];
      // create array with faces
      // have to convert from Assimp format to array
      unsigned int *faceArray;
      faceArray = (unsigned int *)malloc(sizeof(unsigned int) * mesh->mNumFaces * 3);
      unsigned int faceIndex = 0;

      conts = BufferContents(mesh->mNumVertices,mesh->mNumFaces);
      firstComp = 0;
      for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
         const aiFace* face = &mesh->mFaces[t];
         memcpy(&faceArray[faceIndex], face->mIndices,3 * sizeof(unsigned int));
         if (face->mIndices[2] > mesh->mNumVertices){// || face->mIndices[2] < 0) {
            faceArray[faceIndex+2] = faceArray[faceIndex+1]+1;
         }
         conts.faces.push_back(glm::vec3((float)faceArray[faceIndex],(float)faceArray[faceIndex+1],(float)faceArray[faceIndex+2]));

         faceIndex += 3;
      }
      aMesh.numFaces = sc->mMeshes[n]->mNumFaces*3;

      // generate Vertex Array for mesh 
      // not gonna use vaos though
      //glGenVertexArrays(1,&(aMesh.vao));
      //glBindVertexArray(aMesh.vao);

      // buffer for faces
      glGenBuffers(1, &(aMesh.ibo));
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aMesh.ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, faceArray, GL_STATIC_DRAW);

      // buffer for vertex positions
      if (mesh->HasPositions()) {
         glGenBuffers(1, &(aMesh.vbo));
         glBindBuffer(GL_ARRAY_BUFFER, aMesh.vbo);
         glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
         for (int i = 0; i < mesh->mNumVertices; i++) {
            conts.verts.push_back(glm::vec3(mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z));
         }
      }

      // buffer for vertex normals
      if (mesh->HasNormals()) {
         glGenBuffers(1, &(aMesh.nbo));
         glBindBuffer(GL_ARRAY_BUFFER, aMesh.nbo);
         glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
         for (int i = 0; i < mesh->mNumVertices; i++) {
            conts.norms.push_back(glm::vec3(mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z));
         }
      }

      // buffer for vertex texture coordinates
      if (mesh->HasTextureCoords(0)) {
         float *texCoords = (float *)malloc(sizeof(float)*2*mesh->mNumVertices);
         for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {

            texCoords[k*2]   = mesh->mTextureCoords[0][k].x;
            texCoords[k*2+1] = mesh->mTextureCoords[0][k].y;
            conts.texes.push_back(glm::vec2(mesh->mTextureCoords[0][k].x,mesh->mTextureCoords[0][k].y));

         }
         glGenBuffers(1, &(aMesh.tbo));
         glBindBuffer(GL_ARRAY_BUFFER, aMesh.tbo);
         glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
      }

      // buffer for bones
      /*if (mesh->HasBones()) {
         glGenBuffers(1, &(aMesh.bbo));
         glBindBuffer(GL_ARRAY_BUFFER, aMesh.bbo);
         for (unsigned int b = 0; b < mesh->mNumBones; b++) {
      */

      // unbind buffers
      //glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER,0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
      //conts.print();
      // create material uniform buffer
      aiMaterial *mtl = sc->mMaterials[mesh->mMaterialIndex];

      aiString texPath; //contains filename of texture
      if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)){
            //bind texture
            unsigned int texId = textureIdMap[texPath.data];
            aMesh.texIndex = texId;
            aMat.texCount = 1;
         }
      else
         aMat.texCount = 0;

      float c[4];
      set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
      aiColor4D diffuse;
      if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
         color4_to_float4(&diffuse, c);
      memcpy(aMat.diffuse, c, sizeof(c));

      set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
      aiColor4D ambient;
      if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
         color4_to_float4(&ambient, c);
      memcpy(aMat.ambient, c, sizeof(c));

      set_float4(c, 0.1f, 0.1f, 0.15f, 1.0f);
      aiColor4D specular;
      if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
         color4_to_float4(&specular, c);
      memcpy(aMat.specular, c, sizeof(c));

      set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
      aiColor4D emission;
      if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
         color4_to_float4(&emission, c);
      memcpy(aMat.emissive, c, sizeof(c));

      float shininess = 0.0;
      unsigned int max;
      aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
      aMat.shininess = shininess;
      aMesh.mat = aMat;
      //glGenBuffers(1,&(aMesh.uniformBlockIndex));
      //glBindBuffer(GL_UNIFORM_BUFFER,aMesh.uniformBlockIndex);
      //glBufferData(GL_UNIFORM_BUFFER, sizeof(aMat), (void *)(&aMat), GL_STATIC_DRAW);

      meshData.push_back(aMesh);
      contents.push_back(conts);
   }
}


