//
//  Bjorn.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/3/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#include "Bjorn.h"


#ifdef __APPLE__
#include <sys/time.h>
#endif

#include "Mountain.h"
#include "../Utils/World.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "../Utils/GLSL_helper.h"

#define HEIGHT 0
#define MAX_SPEED 2

Bjorn::~Bjorn()
{
}

Bjorn::Bjorn()
{
   
}

Bjorn::Bjorn(glm::vec3 pos, GLHandles hand, GameModel *model, World world) :
   GameObject("Bjorn")
{
   initialize(model, 0, 0, hand);
   setPos(pos + glm::vec3(0.0, 0.75f, 0.5f));
   setScale(glm::vec3(0.1f));
   setRotation(glm::vec3(0.0, -90.0, 0.0));
   setVelocity(glm::vec3(0));
   jumping = false;
   suspended = false;
   gravity = -3;
   mass = 20;
   this->world = world;
}


void Bjorn::step()
{
	double curtime = glfwGetTime();
   float deltaT = (float)(curtime -  lastUpdated);
   //Update position based on velocity
   moveBy(deltaT * getVel());
   //Fall due to gravity if not colliding with anything, this is a weird y offset, i don't get it
   if(world.detectCollision(glm::vec3(getPos().x, getPos().y + .09, getPos().z)) == 0 && !suspended)
      addVelocity(glm::vec3(0.0, ((mass * gravity) * .002f), 0.0));
   else
   {
      //setPos(glm::vec3(getPos().x,world.getY(getPos()),getPos().z));
      jumping = false;
      setVelocity(glm::vec3(getVel().x, 0, getVel().z));
      //Update X velocity due to friction
      if(getVel().x > 0.1)
         addVelocity(glm::vec3(-0.15,0.0,0.0));
      else if (getVel().x < -0.1)
         addVelocity(glm::vec3(0.15, 0.0,0.0));
      else
         setVelocity(glm::vec3(0.0, getVel().y, getVel().z));
   }
   
   setPos(glm::vec3(getPos().x, getPos().y, Mountain::getZ(getPos()) - 1));
   lastUpdated = curtime;
   
   return;
}

void Bjorn::moveRight()
{
   if(getVel().x > -MAX_SPEED)
      addVelocity(glm::vec3(-0.5, 0.0, 0.0));
}

void Bjorn::moveLeft()
{
   if(getVel().x < MAX_SPEED)
      addVelocity(glm::vec3(0.5f,0.0,0.0));
}

void Bjorn::jump()
{
   if(!jumping || DEBUG_GAME)
   {
      setVelocity(glm::vec3(getVel().x,2.5,getVel().z));
      jumping = true;
   }
}

void Bjorn::suspend()
{
   setVelocity(glm::vec3(0));
   suspended = true;
}

void Bjorn::unsuspend()
{
   suspended = false;
}

void Bjorn::launch(float angle)
{
   jumping = true;
   setVelocity(glm::vec3(-5 * cos(angle), 5 * sin(angle), 0.0));
//   addVelocity(glm::vec3(-2.5 * cos(angle), 2.5 * sin(angle), 0.0));
}

