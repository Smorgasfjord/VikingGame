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
   moveBy(glm::vec3(0, 0, .25));
   scaleBy(glm::vec3(.3f));
   rotateBy(glm::vec3(0, 180, -90));
   this->world = world;
   bjorn = character;
   previousAngle = glm::vec3(0.0f);
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
   //Vector along which the hammer should end up
   glm::vec3 currentAngle = getRot();
   //Angle between desired vector and neutral hammer position (straight up)
   float angle = atan2(x, y);
   if(!isnan(angle) && !collision)
   {
      //Save the last angle
      previousAngle = currentAngle;
      //Rotation is flipped if the hammer is flipped
      if(bjorn->mountainSide == MOUNT_FRONT || bjorn->mountainSide == MOUNT_BACK)
      {
         
         if(((int)getRot().y % 360) == 180)
            setRotation(glm::vec3(currentAngle.x, currentAngle.y, -angle * (180.0 / pi)));
         else
            setRotation(glm::vec3(currentAngle.x, currentAngle.y, angle * (180.0 / pi)));
      }
      else
      {
         if(((int)getRot().y % 360) == 180)
            setRotation(glm::vec3(angle * (180.0 / pi), currentAngle.y, currentAngle.z));
         else
            setRotation(glm::vec3(-angle * (180.0 / pi), currentAngle.y, currentAngle.z));
      }
   }
}


void Hammer::flip()
{
   if (!collision) {
      hammerSide = !hammerSide;
      rotateBy(glm::vec3(0, 180, 0));
   }
}

// What hammer does
void Hammer::step(double timeStep)
{
   setVelocity(bjorn->getVel());
   moveBy(getVel()*(float)timeStep);
   setPos(bjorn->getPos());

   //Move the hammer to the proper position
   if(bjorn->mountainSide == MOUNT_FRONT)
      moveBy(glm::vec3(0, 0, -.2));
   else if(bjorn->mountainSide == MOUNT_RIGHT)
      moveBy(glm::vec3(-.2, 0, 0));
   else if(bjorn->mountainSide == MOUNT_BACK)
      moveBy(glm::vec3(0, 0, .2));
   else
      moveBy(glm::vec3(.2, 0, 0));

   //Update hammer rotation if we're on a different side of the mountain
   if(mountainSide != bjorn->mountainSide)
   {
      if(mountainSide < bjorn->mountainSide)
         rotateBy(glm::vec3(0, 90, 0));
      else
         rotateBy(glm::vec3(0, -90, 0));
      mountainSide = bjorn->mountainSide;
   }
   return;
}

// How the world reacts to Hommur
void Hammer::update(double timeStep) {
   glm::vec3 hammerTip, movedAngle;
   CollisionData dat;
   dat = world->checkCollision(this, modelIdx);
   if (dat.hitObj.obj >= 0) {
      /*
      printf("Hommur vertex %d hit platform %d face %d at the location (%f, %f, %f) with normal (%f, %f, %f) while moving in the direction (%f, %f, %f) after trying to move (%f, %f, %f)\n",
                hammer.model.children[dat.thisObj.nod].name.c_str(), dat.thisObj.tri, dat.hitObj.obj, dat.hitObj.tri,
                dat.collisionPoint.x, dat.collisionPoint.y,dat.collisionPoint.z,dat.collisionNormal.x, dat.collisionNormal.y,dat.collisionNormal.z,
                dat.collisionAngle.x, dat.collisionAngle.y,dat.collisionAngle.z,dat.collisionStrength.x, dat.collisionStrength.y,dat.collisionStrength.z);
      */
      if (hammerSide) { 
         //m/s         = m/s           
         activeForce = dat.collisionStrength * (float)(GRAVITY * .75f);
         //printf("Collision Strength (%f %f %f)\n", activeForce.x, activeForce.y, activeForce.z);
         //                  m          /       s
         bjorn->addVelocity(-activeForce);//(float)timeStep);
         Sound::hammerSmash();
      }
      else if (!hammerSide && !collision) {
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         activeForce = dat.collisionStrength * dat.collisionNormal*(float)GRAVITY/2.0f;
         bjorn->setVelocity(bjorn->getVel() * dat.collisionNormal);
         pickAngle = glm::normalize(dat.collisionAngle);
      }
      else {
         //m/s         = m/s                     - (m/s^2                * s)
         activeForce = dat.collisionStrength*(float)GRAVITY/2.0f; 
         //                  m/s
         bjorn->addVelocity(-activeForce);//(float)timeStep);
      }
      collision = true;
      //bjorn->suspend();
      movedAngle = getRot();
      setRotation(previousAngle);
      if (dat.collisionStrength.x == 0.0f) dat.collisionStrength.x = 1.0f;
      if (dat.collisionStrength.y == 0.0f) dat.collisionStrength.y = 1.0f;
      if (dat.collisionStrength.z == 0.0f) dat.collisionStrength.z = 1.0f;
   }
   else {
      bjorn->unsuspend();
      collision = false;
   }

   hammerTip = getPos();
   hammerTip.y += sin(d2r(getRot().z + 90.0));
   if(!hammerSide)
      hammerTip.x -= cos(d2r(getRot().z + 90.0));
   else
      hammerTip.x += cos(d2r(getRot().z + 90.0));
}
