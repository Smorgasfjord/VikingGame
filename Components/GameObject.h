
#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#ifdef __APPLE__
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "../glm/gtx/vector_query.hpp" //
#endif

#ifdef __unix__
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "../glm/gtx/vector_query.hpp" //
#endif

#include "GameModel.h"
#include "../Utils/GLSL_helper.h"
#include "../Utils/GLHandles.h"
#include "../Utils/CMeshLoaderSimple.h"
#include <stdlib.h>
#include <string>

using namespace std;

typedef struct trans_data Transform_t;
struct trans_data {
   glm::vec3 pos;
   glm::vec3 scale;
   glm::vec3 orient;
   glm::vec3 velocity;
   glm::mat4 translate;
   glm::mat4 scaling;
   glm::mat4 rotation;
   glm::mat4 transform;
};

void initGameObjState(Transform_t *state);
void setMaterial(int i, int ShadeProg);

class ObjectMesh {
   public:
      int meshIdx;
      MeshBufferData buffDat;

      ObjectMesh() 
      {
      }
      
      ObjectMesh(int m, MeshBufferData n) :
         meshIdx(m),
         buffDat(n)
      {
      }

      void render(GLHandles handle);
};

class ObjectNode {
   public:
      vector<ObjectMesh> meshes;
      vector<ObjectNode> children;
      string name;
      Transform_t state;

      ObjectNode()
      {
         initGameObjState(&state);
      }

      ObjectNode(string n) :
        name(n) 
      {
         initGameObjState(&state);
      }
      
      void initialize(ModelNode *modNod);
      void render(GLHandles handle, glm::mat4 cumulative);
};

class GameObject {
   public:
      ObjectNode model;
      string name;
      int modelIdx;
      int collisionGroup;
      int grounded;
      int gravityAffected;
      float mass;
      GLHandles handles;

      GameObject()
      {
      }

      GameObject(string n) :
         name(n)
      {
         grounded = 0;
         gravityAffected = 1;
         mass = 1.0;
         collisionGroup = 0;
      }
     
      void initialize(GameModel *gMod, int modIdx, int collGroup, GLHandles handle);
      //Physics
      void setPhysProps(float mass, int gravAffect);
   
      glm::vec3 checkCollision(GameObject other);
      glm::vec3 applyForce(glm::vec3 force);
      glm::vec3 addVelocity(glm::vec3 vel);
      glm::vec3 setVelocity(glm::vec3 vel);
      float scaleMass(float scale);
      //Get object properties
      glm::vec3 getPos();
      glm::vec3 getVel();
      glm::vec3 getRot();
      //Update object
      void moveBy(glm::vec3 amount);
      void scaleBy(glm::vec3 amount);
      void rotateBy(glm::vec3 amount);
      //Set object
      void setPos(glm::vec3 pos);
      void setScale(glm::vec3 scale);
      void setRotation(glm::vec3 rot);
      //Drawing
      void draw();
      virtual void update(double timeStep);
      glm::vec3 applyTransform(glm::mat4 tran);
   private:
      void updateTransformMatrix();
};

#endif
