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
#include "../GameObject.h"
#include "Bjorn.h"

class Hammer : public GameObject
{
public:
   void step();
   Hammer() {}
   Hammer(std::string name);
   ~Hammer();
   //Hammer(GLHandles hand, Model model, World world, Bjorn *character);
   void setInWorld(World world, Bjorn *character);
   void updatePos(float dx, float dy);
   void updateAngle(float x, float y);
private:
   World world;
   Model mod;
   Bjorn *bjorn;
   float rotation;
   int flipped;
   bool collision;
   //float mass;
   float previousAngle;
};

#endif /* defined(__MyGame__Hammer__) */
