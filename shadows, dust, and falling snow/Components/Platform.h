//
//  Platform.h
//  levelBuilder
//
//  Created by Taylor Woods on 4/15/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef __levelBuilder__Platform__
#define __levelBuilder__Platform__

#include <iostream>
#include <vector>
#include "GameObject.h"
#include </home/nclarke/Desktop/deps/glm/include/glm/glm.hpp>
#include "../Utils/GLHandles.h"

#define STEP .25

class Platform : public GameObject
{
   public:
      void step();
      ~Platform() {}
      Platform();
      Platform(glm::vec3 pos, GLHandles hand, GameModel *model);
      Platform(glm::vec3 pos, glm::vec3 size, float rotation, int mountSide, GLHandles hand, GameModel *model);
      bool detectCollision(glm::vec3 pos);
      void checkSide();
      void stretch();
      void shrink();
      string toString();
      int mountainSide;
      void moveUp();
      void moveDown();
      void moveLeft();
      void moveRight();
      static std::vector<Platform> importLevel(std::string const & fileName, GLHandles handles, GameModel *platMod);
};

#endif /* defined(__levelBuilder__Platform__) */
