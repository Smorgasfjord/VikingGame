#ifndef __CHUNKS_H__
#define __CHUNKS_H__

#include <vector>
#include <map>

#include "../Components/GameObject.h"
#include "../Models/GameModel.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr

#define CHUNK_SIZE 8.0f
#define APP 0.001f
#define BOOST 1.0001f

using namespace std;

class ChunkWorld;
class Chunk;
class MicroChunk;
typedef struct chunk_data ChunkData;
typedef struct obj_data ObjData;

struct chunk_data {
   int x; //x index
   int y; //y index
   int z; //z index
};

struct obj_data {
   int obj; //object index
   int nod; //object node
   int mesh; //mesh index
   int tri; //face index
};

// for mapping purposes
bool operator<(const ChunkData a, const ChunkData b) {
   if (a.x != b.x)
      return a.x < b.x;
   if (a.y != b.y)
      return a.y < b.y;
   return a.z < b.z;
}
bool operator==(const ChunkData a, const ChunkData b) {
   return a.x == b.x && a.y == b.y && a.z == b.z;
}
bool operator!=(const ChunkData a, const ChunkData b) {
   return !(a==b);
}
bool operator<(const ObjData a, const ObjData b) {
   if (a.obj != b.obj)
      return a.obj < b.obj;
   if (a.nod != b.nod)
      return a.nod < b.nod;
   if (a.mesh != b.mesh)
      return a.mesh < b.mesh;
   return a.tri < b.tri;
}
bool operator==(const ObjData a, const ObjData b) {
   return a.obj == b.obj && a.nod == b.nod && a.mesh == b.mesh && a.tri == b.tri;
}
bool operator!=(const ObjData a, const ObjData b) {
   return !(a==b);
}

glm::vec3 nextChunk(glm::vec3 pos, glm::vec3 ray, float scale);

class MicroChunk {
   public:
      map<ObjData, glm::vec3> objects;
      glm::vec3 minBound, maxBound, index;
      int valid;

      MicroChunk() :
         valid(0)
      {
      }

      MicroChunk(float x, float y, float z) :
         valid(1),
         index((int)x,(int)y,(int)z),
         minBound(x/CHUNK_SIZE,y/CHUNK_SIZE,z/CHUNK_SIZE),
         maxBound((x+1.0)/CHUNK_SIZE, (y+1.0)/CHUNK_SIZE,
                  (z+1.0)/CHUNK_SIZE)
      {
      }

      int isValid() {
         return valid;
      }
};

class Chunk {
   public:
      int empty;
      glm::vec3 minBound, maxBound, index;

      Chunk() :
         index(0.0,0.0,0.0),
         minBound(0.0,0.0,0.0),
         maxBound(0.0,0.0,0.0),
         empty(1)
      {
      }

      Chunk(float x, float y, float z) :
         index(x, y, z),
         minBound(x, y, z),
         maxBound(x+1.0, y+1.0, z+1.0),
         empty(0)
      {
      }

      int isValid() {
         return !empty;
      }

};

class ChunkWorld {
   public:
      map<ChunkData, Chunk> chunkMap;
      map<ChunkData, MicroChunk> uChunkMap;
      Chunk invalidC;
      MicroChunk invalidM;
      glm::vec3 scale;
      vector<GameObject> objects;
      vector< vector<BufferContents> > models;
//      vector<LightSource> lights;
      int objCount;

      ChunkWorld() :
         objCount(0)
      {
      }

      ChunkWorld(const int & width, const int & height, const int & depth) :
         scale(width, height, depth),
         objCount(0)
      {
      }
      
      void print(); 
      Chunk & findChunk(float x, float y, float z);      
      Chunk & addChunk(float x, float y, float z);
      MicroChunk * findMicroChunk(float x, float y, float z); 
      MicroChunk * addMicroChunk(float x, float y, float z);
      void traceLine(glm::vec3 start, glm::vec3 end, ObjData dat);      
      void traceTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, ObjData check);
      void traceMesh(const BufferContents & geom, glm::mat4 trans, ObjData dat);
      int traceNode(ObjectNode *nod, const vector<BufferContents> & geom, glm::mat4 trans, ObjData dat); 
      int populate(GameObject *obj, const std::vector<BufferContents> & geom);
      void repopulate(GameObject *obj, int objIndex);
      int checkForCollision(GameObject *obj, int objIndex, glm::mat4 transform);
      void depopulate(int objIndex);
      //int addMaterial(mat_t mat);      
      //int addLight(LightSource light);
};


#endif
