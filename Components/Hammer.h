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
#include "../Components/GameObject.h"
#include "Bjorn.h"
#include "../Audio/Sound.h"

class Hammer : public GameObject
{
public:
   void step(double timeStep);
   void update(double timeStep);
   Hammer() 
   {
   }
   Hammer(std::string name);
   ~Hammer();
   //Hammer(GLHandles hand, Model model, World world, Bjorn *character);
   void setInWorld(World * world, Bjorn *character, GameModel *mod, GLHandles hand);
   void updatePos(float dx, float dy);
   void updateAngle(float x, float y);
   void flip();
private:
   World * world;
   Model mod;
   Bjorn *bjorn;
   glm::vec3 pickAngle;
   float rotation;
   bool hammerSide;
   bool collision;
   //float mass;
   float previousAngle;
};

#endif /* defined(__MyGame__Hammer__) */
