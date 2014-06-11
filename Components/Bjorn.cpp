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

#define MAX_SPEED 2.5
#define RIGHT_ARM 2

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
   initialize(*model, 0, 0, hand);
   setPos(pos + glm::vec3(5, 1.95f, 0.5f));
   setScale(glm::vec3(0.1f));
   setRotation(glm::vec3(0.0, 270.0, 0.0));
   setVelocity(glm::vec3(0));
   jumping = false;
   grounded = true;
   suspended = false;
   facingRight = true;
   screamed = false;
   gravity = -3;
   mass = 20;
   this->world = worl;
   modelIdx = world->placeObject(this, &simple);
   mountainSide = Mountain::getSide(pos);
   animationStage = 0;
}

void Bjorn::save() {
   s_state = getState();
   s_facingRight = facingRight;
   s_mountainSide = mountainSide;
}
void Bjorn::reset() {
   setState(s_state);
   facingRight = s_facingRight;
   mountainSide = s_mountainSide;
   setVelocity(glm::vec3(0.0f));
   grounded = true;
}

void Bjorn::animate(ObjectNode & nod, glm::mat4 cumul) {
   float frac, idx;
   int keyIdx;
   if (nod.keys.size() > 0) {
      frac = modf(animationStage*2.0,&idx);
      keyIdx = (int)idx % (nod.keys.size()-1);
      nod.state.pos = (nod.keys[keyIdx].t*(1.0f-frac) + nod.keys[keyIdx+1].t*frac);
      nod.state.scale = (nod.keys[keyIdx].s*(1.0f-frac) + nod.keys[keyIdx+1].s*frac);
      nod.state.orient = (nod.keys[keyIdx].r*(1.0f-frac) + nod.keys[keyIdx+1].r*frac);
      nod.state.translate = glm::translate(glm::mat4(1.0f), nod.state.pos);
      nod.state.scaling = glm::scale(glm::mat4(1.0f), nod.state.scale);
      nod.state.rotation = glm::rotate(glm::mat4(1.0f), nod.state.orient.z*180.0f/(float)M_PI-90.0f, glm::vec3(1.0f,0.0,0.0));
      nod.state.rotation *= glm::rotate(glm::mat4(1.0f), nod.state.orient.y*180.0f/(float)M_PI, glm::vec3(0.0,1.0f,0.0));
      nod.state.rotation *= glm::rotate(glm::mat4(1.0f), nod.state.orient.x*180.0f/(float)M_PI, glm::vec3(0.0,0.0,1.0f));
      nod.state.transform = nod.state.translate * nod.state.rotation * nod.state.scaling;
   }
   for (int i = 0; i < nod.children.size(); i++) {
      animate(nod.children[i],cumul);
   }
}

//What Bjorn does
void Bjorn::step(double timeStep)
{
   int newSide;
   if (grounded || !jumping) {
      Sound::walk();
      //Update X velocity due to friction
      if(glm::length(getVel()) > 0.1)
      {
         animationStage += timeStep * glm::length(getVel());
         setVelocity(getVel() * (float)exp(-1.0 * timeStep));
      }
      else
      {
         setVelocity(glm::vec3(0.0f));
         animationStage = 0;
         Sound::stopWalk();
      }
   }
   else {
      if(glm::length(getVel()) > 0.1)
      {
         setVelocity(getVel() * (float)exp(-0.2 * timeStep));
      } 
      animationStage = 0;
      Sound::stopWalk();
   }
   animate(model, glm::mat4(1.0f));
   //                         (m/s^2  * s)
   if (!suspended) addVelocity(-glm::vec3(0.0,GRAVITY * timeStep,0.0));
   moveBy(getVel()*(float)timeStep);
   
   glm::vec3 newPos = getPos();
   newSide = Mountain::getSide(newPos);
   //Check if we've changed sides of the mountain
   if(newSide != mountainSide)
   {
      setRotation(glm::vec3(0, 45.0 * newSide + 270.0, 0));
      if (!facingRight) rotateBy(glm::vec3(0,180.0f,0));
      
      mountainSide = newSide;
   }
   
   return;
}

//How the world reacts to what Bjorn does
void Bjorn::update(double timeStep) {
   CollisionData cData;
   GameObject collidedWith;
   static double jumpCount = 0.0;
   glm::vec3 displacement, newPos, adjustment;
   cData = world->checkCollision(this, modelIdx);
   if (cData.hitObj.obj >= 0) {
      displacement = cData.collisionAngle-cData.collisionStrength;
      moveBy(displacement); //reevaluate location

      collidedWith = world->getObjectByIndex(cData.hitObj.obj);
      //Nudge bjorn towards the correct position on the platform
      newPos = Mountain::lockOn(getPos(),displacement);
      adjustment = ((newPos + displacement * glm::vec3(-1.0f,0.0f,-1.0f)) - getPos());
      moveBy(adjustment/(glm::length(adjustment)+1.0f));
      
      setVelocity((getVel() * 0.5f + glm::reflect(getVel(), cData.collisionNormal)) / 2.0f + cData.collisionNormal * (float)timeStep * 0.1f);
      if (cData.collisionNormal.y > 0.5) {
         jumpCount = 0.0;
         jumping = false;
         grounded = true;
      }
      else jumping = true;
      
   }
   else {
      if (jumpCount < 0.8) {
         jumpCount+=timeStep;
      }
      else {
         grounded = false;
         jumping = true;
         newPos = Mountain::lockOn(getPos(),displacement);
         moveBy(((newPos+displacement*glm::vec3(-1.0f+(0.2*getVel().y),0.0f,-1.0f+(0.2*getVel().y))) - getPos())/10.0f);
      }
   }
}

void Bjorn::moveRight(float timeStep)
{
   glm::vec4 speed;
   if(!facingRight)
   {
      rotateBy(glm::vec3(0, 180.0, 0));
      facingRight = true;
   }

   if (grounded || DEBUG_GAME) {
      speed = getRotMat() * glm::vec4(0.0f, (float)GRAVITY*1.05f, 30.0f, 0.0f) * timeStep + glm::vec4(0.0f,-getVel().y,0.0f,0.0);
   }
   else {
      speed = getRotMat() * glm::vec4(0.0f, 0.0f, 6.0f, 0.0f) * timeStep;
   }
   if (glm::length(getVel()*glm::vec3(1.0f,0.0,1.0f) + glm::vec3(speed)*glm::vec3(1.0f,0.0,1.0f)) < MAX_SPEED) {
      addVelocity(speed.xyz());
   }
   else {
      addVelocity(glm::vec3(0.0,speed.y,0.0) + glm::vec3(speed.x,0.0,speed.z)*((float)MAX_SPEED-min(glm::length(getVel()*glm::vec3(1.0f,0.0,1.0f)),(float)MAX_SPEED)));
   }
}

void Bjorn::moveLeft(float timeStep)
{
   glm::vec4 speed;
   if(facingRight)
   {
      rotateBy(glm::vec3(0, 180.0, 0));
      facingRight = false;
   }

   if (grounded || DEBUG_GAME) {
      speed = getRotMat() * glm::vec4(0.0f, (float)GRAVITY*1.05f, 30.0f, 0.0f) * timeStep + glm::vec4(0.0f,-getVel().y,0.0f,0.0);
   }
   else {
      speed = getRotMat() * glm::vec4(0.0f, 0.0f, 6.0f, 0) * timeStep;
   }

   if (glm::length(getVel()*glm::vec3(1.0f,0.0,1.0f) + glm::vec3(speed)*glm::vec3(1.0f,0.0,1.0f)) < MAX_SPEED) {
      addVelocity(speed.xyz());
   }
   else {
      addVelocity(glm::vec3(0.0,speed.y,0.0) + glm::vec3(speed.x,0.0,speed.z)*((float)MAX_SPEED-min(glm::length(getVel()*glm::vec3(1.0f,0.0,1.0f)),(float)MAX_SPEED)));
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
