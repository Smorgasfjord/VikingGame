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

void Hammer::setInWorld(World * world, Bjorn *character, GameModel *hammerMod, GLHandles handles)
{
   GameModel simple = genSimpleModel(hammerMod);
   initialize(hammerMod, 0, 0, handles);
   setPos(character->getPos());
   moveBy(glm::vec3(0, .5, .25));
   scaleBy(glm::vec3(.3f));
   rotateBy(glm::vec3(0, 180, -90));
   this->world = world;
   bjorn = character;
   previousAngle = 0.0;
   hammerSide = true;
   modelIdx = world->placeObject(this, &simple);
   mountainSide = bjorn->mountainSide;
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
      if(bjorn->mountainSide == MOUNT_FRONT || bjorn->mountainSide == MOUNT_BACK)
      {
         if(hammerSide)
            setRotation(glm::vec3(currentAngle.x, currentAngle.y, -angle * (180.0 / pi)));
         else
            setRotation(glm::vec3(currentAngle.x, currentAngle.y, angle * (180.0 / pi)));
      }
      if(!hammerSide)
         setRotation(glm::vec3(currentAngle.x, currentAngle.y, -angle  * (180.0 / pi)));
      else
      {
         if(hammerSide)
            setRotation(glm::vec3(angle * (180.0 / pi), currentAngle.y, currentAngle.z));
         else
            setRotation(glm::vec3(-angle * (180.0 / pi), currentAngle.y, currentAngle.z));
      }
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
   setPos(bjorn->getPos());

   //Move the hammer to the proper position
   if(bjorn->mountainSide == MOUNT_FRONT)
      moveBy(glm::vec3(0, .5, -.2));
   else if(bjorn->mountainSide == MOUNT_RIGHT)
      moveBy(glm::vec3(-.2, .5, 0));
   else if(bjorn->mountainSide == MOUNT_BACK)
      moveBy(glm::vec3(0, .5, .2));
   else
      moveBy(glm::vec3(.2, .5, 0));
   
   //Update hammer rotation if we're on a different side of the mountain
   if(mountainSide != bjorn->mountainSide)
   {
      if(mountainSide < bjorn->mountainSide)
         rotateBy(glm::vec3(0, 90, 0));
      else
         rotateBy(glm::vec3(0, -90, 0));
      mountainSide = bjorn->mountainSide;
   }
   
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
   dat = world->checkCollision(this, modelIdx);
   if (dat.hitObj.obj >= 0) {
      printf("Hommur vertex %d hit platform %d face %d at the location (%f, %f, %f) with normal (%f, %f, %f) while moving in the direction (%f, %f, %f)\n",
                /*hammer.model.children[dat.thisObj.nod].name.c_str(), */dat.thisObj.tri, dat.hitObj.obj, dat.hitObj.tri,
                dat.collisionPoint.x, dat.collisionPoint.y,dat.collisionPoint.z,dat.collisionNormal.x, dat.collisionNormal.y,dat.collisionNormal.z,
                dat.collisionAngle.x, dat.collisionAngle.y,dat.collisionAngle.z);

      if (hammerSide) { 
         //m         = m                     * (no unit)           - (m/s^2                * s^2)
         activeForce = dat.collisionStrength * dat.collisionNormal - glm::vec3(0.0f,GRAVITY*timeStep*timeStep,0.0);
         //                  m          /       s
         bjorn->addVelocity(-activeForce);//(float)timeStep);
         collision = false;
         Sound::hammerSmash();
      }
      else if (!hammerSide && !collision) {
         //m         = m                     * (no unit)           - (m/s^2                * s^2)
         activeForce = dat.collisionStrength * dat.collisionNormal - glm::vec3(0.0f,GRAVITY*timeStep*timeStep,0.0);
         bjorn->setVelocity(bjorn->getVel() * dat.collisionNormal);
         pickAngle = glm::normalize(dat.collisionAngle);
         collision = true;
      }
      else {
         //m         = m                     - (m/s^2                * s^2)
         activeForce = dat.collisionStrength - glm::vec3(0.0f,GRAVITY*timeStep*timeStep,0.0); 
         //                  m          /       s
         bjorn->addVelocity(-activeForce);//(float)timeStep);
      }
      //addVelocity((dat.collisionAngle-dat.collisionStrength)/(float)timeStep);
   }
   else {
      collision = false;
   }
   moveBy(getVel()*(float)timeStep);
   setPos(glm::vec3(bjorn->getPos().x,bjorn->getPos().y + 0.6f,bjorn->getPos().z));
   hammerTip = getPos();
   hammerTip.y += sin(d2r(getRot().z + 90.0));
   if(!hammerSide)
      hammerTip.x -= cos(d2r(getRot().z + 90.0));
   else
      hammerTip.x += cos(d2r(getRot().z + 90.0));
   updateAngle(hammerTip.x,hammerTip.y);
}
