//
//  Mountain.h
//  levelBuilder
//
//  Created by Taylor Woods on 4/16/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef __levelBuilder__Mountain__
#define __levelBuilder__Mountain__

#include <iostream>
#include "GameObject.h"
#include "../Utils/GLHandles.h"
#include "../Models/GameModel.h"
#include "../glm/glm.hpp"

#define MOUNT_FRONT 0
#define MOUNT_FR    1
#define MOUNT_RIGHT 2
#define MOUNT_RB   3
#define MOUNT_BACK 4
#define MOUNT_BL   5
#define MOUNT_LEFT 6
#define MOUNT_LF   7
#define MOUNT_WIDTH 60.0
#define MOUNT_DEPTH 60.0
#define MOUNT_HEIGHT 50.0

typedef struct plane_t {
   float a;
   float b;
   float c;
   float d;
} Plane_t;

class Mountain : public GameObject
{
   public:
      Mountain();
      Mountain(GLHandles hand, GameModel *model);
      //static float getZ(glm::vec3 pos);
      //static float getX(glm::vec3 pos);
      static int getSide(glm::vec3 pos);
      static glm::vec3 lockOn(glm::vec3 pos, glm::vec3 & norms);
   private:
      static void genPlanes();
      static float testLeftDiagonal(glm::vec3 pos);
      static float testRightDiagonal(glm::vec3 pos);
      static void loadHeightMaps();
};

#endif /* defined(__levelBuilder__Mountain__) */
