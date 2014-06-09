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
   void save();
   void reset();
   int mountainSide;
   bool manualLocked;
private:
   World * world;
   Bjorn *bjorn;
   glm::vec3 pickAngle;
   glm::vec3 pickNormal;
   glm::vec3 bjornOffset;
   glm::vec3 desiredRotation;
   glm::vec3 previousAngle;
   bool hammerSide;
   bool pickCollision;
   bool hammerCollision;
   bool locked;
   glm::vec3 s_desiredRotation;
   glm::vec3 s_previousAngle;
   int s_mountainSide;
   bool s_hammerSide;
   bool s_pickCollision;
   bool s_hammerCollision;
   bool s_locked;
   Transform_t s_state;
   //float mass;
};

#endif /* defined(__MyGame__Hammer__) */
