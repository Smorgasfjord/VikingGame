//
//  Bjorn.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/3/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#define GLM_SWIZZLE
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
#define MAX_SPEED 1.5

Bjorn::~Bjorn()
{
}

Bjorn::Bjorn() 
{

}

Bjorn::Bjorn(glm::vec3 pos, GLHandles hand, GameModel *model, World * worl) :
   GameObject("Bjorn")
{
   GameModel simple = genSimpleModel(model);
   initialize(model, 0, 0, hand);
   setPos(pos + glm::vec3(0.0, 0.95f, 0.5f));
   setScale(glm::vec3(0.1f));
   setRotation(glm::vec3(0.0, -90.0, 0.0));
   setVelocity(glm::vec3(0));
   jumping = false;
   grounded = true;
   suspended = false;
   gravity = -3;
   mass = 20;
   this->world = worl;
   modelIdx = world->placeObject(this, &simple);
   mountainSide = Mountain::getSide(pos);
}

//What Bjorn does
void Bjorn::step(double timeStep)
{
   int newSide;
   if (grounded) {
      Sound::walk();
      //Update X velocity due to friction
      if(glm::length(getVel()) > 0.1)
         setVelocity(getVel() * (float)exp(-1.0 * timeStep));
      else
      {
         setVelocity(glm::vec3(0.0f));
         Sound::stopWalk();
      }
   }
   else {
      Sound::stopWalk();
   }
   //                         (m/s^2  * s)
   addVelocity(-glm::vec3(0.0,GRAVITY * timeStep,0.0));

   moveBy(getVel()*(float)timeStep);
   
   //setPos(Mountain::lockOn(getPos()));
   glm::vec3 newPos = getPos();
   newPos.z += 1.0f;
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
      //if (getVel().x != 0)
         //setVelocity(glm::vec3(0, getVel().y, getVel().x));
      //else
         //setVelocity(glm::vec3(getVel().z, getVel().y, 0));
         
      mountainSide = newSide;
   }
   
   return;
}

//How the world reacts to what Bjorn does
void Bjorn::update(double timeStep) {
   CollisionData dat;
   GameObject collidedWith;
   static double jumpCount = 0.0;
   glm::vec3 thing, newPos;
   dat = world->checkCollision(this, modelIdx);
   if (dat.hitObj.obj >= 0) {
      printf("Bjorn vertex %d hit platform %d face %d at the location (%f, %f, %f) with normal (%f, %f, %f) while moving in the direction (%f, %f, %f) while trying to move (%f, %f, %f)\n",
                /*hammer.model.children[dat.thisObj.nod].name.c_str(), */dat.thisObj.tri, dat.hitObj.obj, dat.hitObj.tri,
                dat.collisionPoint.x, dat.collisionPoint.y,dat.collisionPoint.z,dat.collisionNormal.x, dat.collisionNormal.y,dat.collisionNormal.z,
                dat.collisionAngle.x, dat.collisionAngle.y,dat.collisionAngle.z,dat.collisionStrength.x,dat.collisionStrength.y,dat.collisionStrength.z);
      thing = dat.collisionAngle-dat.collisionStrength + dat.collisionNormal*(float)timeStep*0.0f;
      printf("moving Bjorn (%f, %f, %f)\n", thing.x, thing.y,thing.z);
      moveBy(thing); //reevaluate location
//      moveBy(-getVel()*(float)timeStep); //+ dat.collisionAngle *0.9f); //reevaluate location
      collidedWith = world->getObjectByIndex(dat.hitObj.obj);
      //Nudge bjorn towards the correct position on the platform
      cout << (collidedWith.getPos().z - getPos().z) << " Off from platform center\n";
      cout << "size " << collidedWith.getScale().z << "\n";
      float nudgeAmount = (collidedWith.getPos().z - getPos().z - (collidedWith.getScale().z / 30.0f))* (collidedWith.getScale().z / 50.0f);
      //if(abs(nudgeAmount) > .01)
      //{
         cout << "Moving by " << nudgeAmount << "\n";
      newPos = Mountain::lockOn(getPos(),thing);
      moveBy(((newPos+thing*glm::vec3(-1.0f,0.0f,-1.0f)) - getPos())/30.0f);
         //moveBy(glm::vec3(0, 0, nudgeAmount));
      //}
      
      setVelocity((getVel()*0.5f + glm::reflect(getVel(), dat.collisionNormal))/2.0f + dat.collisionNormal*(float)timeStep*0.1f);
      if (dat.collisionNormal.y > 0.5) {
         jumpCount = 0.0;
         jumping = false;
         grounded = true;
      }
      else jumping = true;

   } 
   else {
      if (jumpCount < 0.2) {
         jumpCount+=timeStep;
      }
      else {
         grounded = false;
         jumping = true;
      }
   }
   newPos = Mountain::lockOn(getPos(),thing);
   moveBy(((newPos+thing*glm::vec3(-1.2f,0.0f,-1.2f)) - getPos())/30.0f);
}

void Bjorn::moveRight()
{
   glm::vec4 speed;
   if (grounded) {
      speed = getRotMat() * glm::vec4(0.0f, 0.1f, 0.5f, 0.0f);
   }
   else {
      speed = getRotMat() * glm::vec4(0.0f, 0.0f, 0.1f, 0.0f);
   }

   if (!suspended && glm::length(getVel()) < MAX_SPEED) {
      addVelocity(speed.xyz());
   }
}

void Bjorn::moveLeft()
{
   glm::vec4 speed;
   if (grounded) {
      speed = getRotMat() * glm::vec4(0.0f, 0.1f, -0.5f, 0);
   }
   else {
      speed = getRotMat() * glm::vec4(0.0f, 0.0f, -0.1f, 0);
   }

   if (!suspended && glm::length(getVel()) < MAX_SPEED) {
      addVelocity(speed.xyz());
   }
}

void Bjorn::jump()
{
   if((!jumping && !suspended) || DEBUG_GAME)
   {
      setVelocity(glm::vec3(getVel().x,GRAVITY/2.0f,getVel().z));
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

