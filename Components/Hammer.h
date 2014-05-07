//
//  Hammer.h
//  MyGame
//
//  Created by Taylor Woods on 5/4/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef __MyGame__Hammer__
#define __MyGame__Hammer__


#ifdef __WIN32__
#include "../glm/glm.hpp"
#else
#include "../glm/glm.hpp"
#endif

#include <iostream>
#include "GameObject.hpp"
#include "Bjorn.h"

class Hammer : public pGameObject
{
public:
   void step();
   void draw();
   Hammer();
   ~Hammer();
   Hammer(GLHandles hand, Model model, World world, Bjorn *character);
   void updatePos(float dx, float dy);
   void updateAngle(float x, float y);
private:
   void SetModel(glm::vec3 loc, glm::vec3 size, float rotation);
   World world;
   Model mod;
   Bjorn *bjorn;
   bool collision;
   float mass;
   float previousAngle;
};

#endif /* defined(__MyGame__Hammer__) */
