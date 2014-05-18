//
//  Hammer.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/4/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#include "Hammer.h"
#define pi 3.14159

Hammer::~Hammer()
{
   
}
Hammer::Hammer(std::string n) : 
   GameObject(n)
{
   
}

void Hammer::setInWorld(World & world, Bjorn *character, GameModel *hammerMod, GLHandles handles)
{
   initialize(hammerMod, 0, 0, handles);
   setPos(character->getPos());
   moveBy(glm::vec3(0, .5, .25));
   scaleBy(glm::vec3(.3f));
   rotateBy(glm::vec3(0, 180, -90));
   this->world = world;
   bjorn = character;
   previousAngle = 0.0;
   hammerSide = true;
   modelIdx = world.placeObject(this, hammerMod);
}

float d2r(float val)
{
   return val * (pi / 180.0f);
}

void Hammer::updatePos(float x, float y)
{
   moveBy(glm::vec3(-.5, 0, 0));
}

void Hammer::updateAngle(float x, float y)
{
   glm::vec2 bjornVec = glm::vec2(bjorn->getPos().x, bjorn->getPos().y);
   //Vector along which the hammer should end up
   glm::vec3 currentAngle = getRot();
   //Angle between desired vector and neutral hammer position (straight up)
   float angle = atan2(x - bjornVec.x, y - bjornVec.y);
   if(!isnan(angle) && !collision)
   {
      //Save the last angle
      previousAngle = currentAngle.z;
      //Rotation is flipped if the hammer is flipped
      if(hammerSide)
         setRotation(glm::vec3(currentAngle.x, currentAngle.y, -angle  * (180.0 / pi)));
      else
         setRotation(glm::vec3(currentAngle.x, currentAngle.y, angle  * (180.0 / pi)));
   }
}


void Hammer::flip()
{
   hammerSide = !hammerSide;
   rotateBy(glm::vec3(0, 180, 0));
}

// What hammer does
void Hammer::step(double timeStep)
{
   setVelocity(bjorn->getVel());
   moveBy(getVel()*(float)timeStep);
   /*glm::vec3 hammerTip = getPos();
   hammerTip.y += sin(d2r(getRot().z + 90));
   if(hammerSide)
      hammerTip.x -= cos(d2r(getRot().z + 90));
   else
      hammerTip.x += cos(d2r(getRot().z + 90));
   if(world.detectCollision(hammerTip))
   {
      if(hammerSide && abs(previousAngle - getRot().z) > 4)
      {
         bjorn->launch(d2r(35));
      }
      else if(!hammerSide)
         bjorn->suspend();
      collision = true;
   }
   else
   {
      bjorn->unsuspend();
   }*/
   return;
}

// How the world reacts to Hommur
void Hammer::update(double timeStep) {
   glm::vec3 hammerTip;
   CollisionData dat;
   dat = world.checkCollision(this, modelIdx);
   if (dat.hitObj.obj >= 0) {
      if (hammerSide) {
         activeForce = dat.collisionStrength * dat.collisionNormal - glm::vec3(0.0f,GRAVITY,0.0);
         bjorn->addVelocity(-activeForce/(float)timeStep);
         collision = false;
         Sound::hammerSmash();
      }
      else if (!hammerSide && !collision) {
         activeForce = dat.collisionStrength * dat.collisionNormal - glm::vec3(0.0f,GRAVITY,0.0);
         bjorn->setVelocity(bjorn->getVel() * dat.collisionNormal);
         pickAngle = glm::normalize(dat.collisionAngle);
         collision = true;
      }
      else {
         activeForce = dat.collisionStrength - glm::vec3(0.0f,GRAVITY,0.0); 
         bjorn->addVelocity(-activeForce/(float)timeStep);
      }
      addVelocity(dat.collisionAngle - dat.collisionStrength);
   }
   else {
      collision = false;
   }
   moveBy(getVel()*(float)timeStep);
   hammerTip = getPos();
   hammerTip.y += sin(d2r(getRot().z + 90.0));
   if(hammerSide)
      hammerTip.x -= cos(d2r(getRot().z + 90.0));
   else
      hammerTip.x += cos(d2r(getRot().z + 90.0));
   updateAngle(hammerTip.x,hammerTip.y);
}
