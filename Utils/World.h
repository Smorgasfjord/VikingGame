//
//  World.h
//  MyGame
//
//  Created by Taylor Woods on 5/4/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef __MyGame__World__
#define __MyGame__World__

#ifdef __APPLE__
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include <iostream>
#include "chunks.h"
#include <vector>
#include "GLHandles.h"
#include "GLSL_helper.h"
#include "../Components/Mountain.h"
#include "../Components/Platform.h"

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr

#define GROUND_MAT 1
#define PLATFORM_COLLISION 1
#define GRAVITY 2.0


class World
{
   public:
      void draw();
      World();
      ~World();
      World(std::vector<Platform> plats, Mountain mnt, GLHandles* handles, int shadeProg);
      int detectCollision(glm::vec3 pos);
      CollisionData checkCollision(GameObject *obj, int objIndex);
      float getY(glm::vec3 pos);
      int placeObject(GameObject *obj, GameModel *mod);
      void updateObject(GameObject *obj, int objIndex);
      void removeObject(int objIndex);
      GameObject & getObjectByIndex(int idx);
      glm::vec3 getStart();
   private:
      int ShadeProg;
      void SetMaterial(int i);
      std::vector<Platform> platforms;
      Mountain mount;
      //std::vector<glm::vec3> groundTiles;
      ChunkWorld space; //rename
      GLHandles* handles;
};

#endif /* defined(__MyGame__World__) */
