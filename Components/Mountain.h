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
#include "GameObject.hpp"
#include "../Utils/GLHandles.h"
#include "../Models/Model.h"
#include "../glm/glm.hpp"

class Mountain : public pGameObject
{
   public:
      void step();
      void draw();
      Mountain();
      Mountain(glm::vec3 pos, GLHandles hand, Model model);
      static float testLeftDiagonal(glm::vec3 pos);
      static float testRightDiagonal(glm::vec3 pos);
      static float getZ(glm::vec3 pos);
      static float getX(glm::vec3 pos);
      float rotation;
   private:
      void SetModel(glm::vec3 loc, glm::vec3 size, float rotation);
      Model mod;
};

#endif /* defined(__levelBuilder__Mountain__) */
