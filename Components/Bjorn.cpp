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
   setRotation(glm::vec3(0.0, 270.0, 0.0));
   setVelocity(glm::vec3(0));
   jumping = false;
   grounded = true;
   suspended = false;
   facingRight = true;
   gravity = -3;
   mass = 20;
   this->world = worl;
   modelIdx = world->placeObject(this, &simple);
   mountainSide = Mountain::getSide(pos);
   cout << "Bjorn is on side " << mountainSide << "\n";
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
   CollisionData cData;
   GameObject collidedWith;
   static double jumpCount = 0.0;
   glm::vec3 displacement, newPos;
   cData = world->checkCollision(this, modelIdx);
   if (cData.hitObj.obj >= 0) {
      /*
      printf("Bjorn vertex %d hit platform %d face %d at the location (%f, %f, %f) with normal (%f, %f, %f) while moving in the direction (%f, %f, %f) while trying to move (%f, %f, %f)\n",
             hammer.model.children[dat.thisObj.nod].name.c_str(), cData.thisObj.tri, cData.hitObj.obj, cData.hitObj.tri,
             cData.collisionPoint.x, cData.collisionPoint.y,cData.collisionPoint.z,cData.collisionNormal.x, cData.collisionNormal.y,cData.collisionNormal.z,
             cData.collisionAngle.x, cData.collisionAngle.y,cData.collisionAngle.z,cData.collisionStrength.x,cData.collisionStrength.y,cData.collisionStrength.z);
      */
      displacement = cData.collisionAngle-cData.collisionStrength;// + cData.collisionNormal*(float)timeStep*0.0f;
      //printf("moving Bjorn (%f, %f, %f)\n", displacement.x, displacement.y,displacement.z);
      moveBy(displacement); //reevaluate location

      collidedWith = world->getObjectByIndex(cData.hitObj.obj);
      //Nudge bjorn towards the correct position on the platform
      newPos = Mountain::lockOn(getPos(),displacement);
      moveBy(((newPos+displacement*glm::vec3(-0.6f,0.0f,-0.6f)) - getPos())/30.0f);
      
      setVelocity((getVel()*0.5f + glm::reflect(getVel(), cData.collisionNormal))/2.0f + cData.collisionNormal*(float)timeStep*0.1f);
      if (cData.collisionNormal.y > 0.5) {
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
   newPos = Mountain::lockOn(getPos(),displacement);
   moveBy(((newPos+displacement*glm::vec3(-0.8f,0.0f,-0.8f)) - getPos())/20.0f);
}

void Bjorn::moveRight()
{
   glm::vec4 speed;
   if(!facingRight)
   {
      rotateBy(glm::vec3(0, 180, 0));
      facingRight = true;
   }

   if (grounded || DEBUG_GAME) {
      speed = getRotMat() * glm::vec4(0.0f, 0.1f, 0.5f, 0.0f);
   }
   else {
      speed = getRotMat() * glm::vec4(0.0f, 0.0f, 0.1f, 0.0f);
   }
   if (glm::length(getVel()) < MAX_SPEED) {
      addVelocity(speed.xyz());
   }
}

void Bjorn::moveLeft()
{
   glm::vec4 speed;
   if(facingRight)
   {
      rotateBy(glm::vec3(0, 180, 0));
      facingRight = false;
   }

   if (grounded || DEBUG_GAME) {
      speed = getRotMat() * glm::vec4(0.0f, 0.1f, 0.5f, 0);
   }
   else {
      speed = getRotMat() * glm::vec4(0.0f, 0.0f, 0.1f, 0);
   }

   if (glm::length(getVel()) < MAX_SPEED) {
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
