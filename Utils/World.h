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
#include <GL/glut.h>
#endif

#include <iostream>
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


class World
{
   public:
      void draw();
      World();
      World(vector<Platform> plats, Mountain mnt, Model gndMod, GLHandles handles, int shadeProg);
      int detectCollision(glm::vec3 pos);
      float getY(glm::vec3 pos);
   private:
      int ShadeProg;
      void setGround(glm::vec3 loc);
      void SetMaterial(int i);
      Model grndMod;
      vector<Platform> platforms;
      Mountain mount;
      vector<glm::vec3> groundTiles;
      GLHandles handles;
};

#endif /* defined(__MyGame__World__) */
