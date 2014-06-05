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
#define MOUNT_RIGHT 1
#define MOUNT_BACK 2
#define MOUNT_LEFT 3
#define MOUNT_WIDTH 60
#define MOUNT_DEPTH 60
#define MOUNT_HEIGHT 44.5

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
      static float testLeftDiagonal(glm::vec3 pos);
      static float testRightDiagonal(glm::vec3 pos);
      static void loadHeightMaps();
};

#endif /* defined(__levelBuilder__Mountain__) */
