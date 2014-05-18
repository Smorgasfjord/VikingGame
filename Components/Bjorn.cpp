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

Bjorn::Bjorn(glm::vec3 pos, GLHandles hand, GameModel *model, World & world) :
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
   mountainSide = Mountain::getSide(pos);
   modelIdx = world.placeObject(this, model);
}

void Bjorn::step()
{
	double curtime = glfwGetTime();
   float deltaT = (float)(curtime -  lastUpdated);
   int newSide;
   //Update position based on velocity
   moveBy(deltaT * getVel());
   //Fall due to gravity if not colliding with anything, this is a weird y offset, i don't get it
   if(world.detectCollision(glm::vec3(getPos().x, getPos().y + .15, getPos().z)) == 0 && !suspended)
   {
      addVelocity(glm::vec3(0.0, ((mass * gravity) * .002f), 0.0));
      Sound::stopWalk();
   }
   else
   {
      Sound::walk();
      jumping = false;
      setVelocity(glm::vec3(getVel().x, 0, getVel().z));
      //Update velocity due to friction
      if(mountainSide == MOUNT_FRONT || mountainSide == MOUNT_BACK)
      {
         if(getVel().x > 0.1)
            addVelocity(glm::vec3(-0.15,0.0,0.0));
         else if (getVel().x < -0.1)
            addVelocity(glm::vec3(0.15, 0.0,0.0));
         else
         {
            setVelocity(glm::vec3(0.0, getVel().y, getVel().z));
            Sound::stopWalk();
         }
      }
      else
      {
         if(getVel().z > 0.1)
            addVelocity(glm::vec3(0, 0, -0.15));
         else if (getVel().z < -0.1)
            addVelocity(glm::vec3(0, 0, 0.15));
         else
         {
            setVelocity(glm::vec3(getVel().x, getVel().y, 0));
            Sound::stopWalk();
         }
      }
   }
   
   //setPos(Mountain::lockOn(getPos()));
   glm::vec3 newPos = getPos();
   newPos.z += 1;
   newSide = Mountain::getSide(newPos);
   //Check if we've changed sides of the mountain
   if(newSide != mountainSide)
   {
      //Moving right around the mountain
      if(newSide > mountainSide)
         rotateBy(glm::vec3(0, 90, 0));
      //Moving left
      else
         rotateBy(glm::vec3(0, -90, 0));
      
      //Transfer velocity
      if (getVel().x != 0)
         setVelocity(glm::vec3(0, getVel().y, getVel().x));
      else
         setVelocity(glm::vec3(getVel().z, getVel().y, 0));
         
      mountainSide = newSide;
         
   }
   lastUpdated = curtime;
   
   return;
}

void Bjorn::moveRight()
{
   if(mountainSide == MOUNT_FRONT)
   {
      if(getVel().x > -MAX_SPEED)
         addVelocity(glm::vec3(-0.5f, 0, 0));
   }
   else if(mountainSide == MOUNT_RIGHT)
   {
      if(getVel().z < MAX_SPEED)
         addVelocity(glm::vec3(0, 0, 0.5f));
   }
   else if(mountainSide == MOUNT_BACK)
   {
      if(getVel().x < MAX_SPEED)
         addVelocity(glm::vec3(0.5f, 0, 0));
   }
   else
   {
      if(getVel().z > -MAX_SPEED)
         addVelocity(glm::vec3(0, 0, -0.5f));
   }
}

void Bjorn::moveLeft()
{
   if(mountainSide == MOUNT_FRONT)
   {
      if(getVel().x < MAX_SPEED)
         addVelocity(glm::vec3(0.5f, 0, 0));
   }
   else if(mountainSide == MOUNT_RIGHT)
   {
      if(getVel().z > -MAX_SPEED)
         addVelocity(glm::vec3(0, 0, -0.5f));
   }
   else if(mountainSide == MOUNT_BACK)
   {
      if(getVel().x > -MAX_SPEED)
         addVelocity(glm::vec3(-0.5f, 0, 0));
   }
   else
   {
      if(getVel().z < MAX_SPEED)
         addVelocity(glm::vec3(0, 0, 0.5f));
   }
}

void Bjorn::jump()
{
   if((!jumping && !suspended) || DEBUG_GAME)
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
}

