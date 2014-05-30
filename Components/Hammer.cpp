//
//  Hammer.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/4/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#include "Hammer.h"
#define pi 3.14159
#define HAMMER_NODE 4
#define PICK_NODE 6

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
   rotateBy(glm::vec3(0, 180, 270));
   this->world = world;
   bjorn = character;
   previousAngle = glm::vec3(0.0f);
   hammerSide = true;
   hammerCollision = pickCollision = false;
   modelIdx = world->placeObject(this, &simple);
   mountainSide = bjorn->mountainSide;
   bjornOffset = glm::vec3(0.0f);
   desiredRotation = previousAngle;
}

float d2r(float val)
{
   return val * (pi / 180.0f);
}

void Hammer::updatePos(float x, float y)
{
   if (!bjorn->facingRight) x *= -1.0;
   bjornOffset = glm::vec3(bjorn->getRotMat() * glm::vec4(-0.2f,y,x,0.0));
   if (glm::length(bjornOffset) > 1.7) {
      bjornOffset *= 1.7f / glm::length(bjornOffset);
   }
}

void Hammer::updateAngle(float x, float y)
{
   //Vector along which the hammer should end up
   glm::vec3 currentAngle = getRot();
   //Angle between desired vector and neutral hammer position (straight up)
   float angle = atan2(x, y);
   if(!isnan(angle) && !(locked || manualLocked))
   {
      //Save the last angle
      previousAngle = currentAngle;
      //Rotation is flipped if the hammer is flipped
      if(bjorn->mountainSide == MOUNT_FRONT || bjorn->mountainSide == MOUNT_BACK)
      {
         
         if(((int)getRot().y % 360) == 180)
            desiredRotation = glm::vec3(currentAngle.x, currentAngle.y, 360.0-angle * (180.0 / pi));
         else
            desiredRotation = glm::vec3(currentAngle.x, currentAngle.y, 360.0 + angle * (180.0 / pi));
      }
      else
      {
         if(((int)getRot().y % 360) == 180)
            desiredRotation = glm::vec3(360.0 + angle * (180.0 / pi), currentAngle.y, currentAngle.z);
         else
            desiredRotation = glm::vec3(360.0-angle * (180.0 / pi), currentAngle.y, currentAngle.z);
      }
   }
}


void Hammer::flip()
{
   if (!hammerCollision && !(locked || manualLocked)) {
      hammerSide = !hammerSide;
      rotateBy(glm::vec3(0, 180, 0));
   }
}

// What hammer does
void Hammer::step(double timeStep)
{
   setVelocity((bjorn->getPos() + bjornOffset - getPos())/((float)timeStep*2.0f));
   if (!hammerCollision) {
      addVelocity(bjorn->getVel());
      //setPos(bjorn->getPos());
      
   }
   if (!locked && !manualLocked) setRotation(desiredRotation);
   moveBy(getVel()*(float)timeStep);
   
   //Update hammer rotation if we're on a different side of the mountain
   if(mountainSide != bjorn->mountainSide)
   {
      if(mountainSide < bjorn->mountainSide)
         rotateBy(glm::vec3(0, 90, 0));
      else
         rotateBy(glm::vec3(0, 270, 0));
      mountainSide = bjorn->mountainSide;
   }
   return;
}

// How the world reacts to Hommur
void Hammer::update(double timeStep) {
   static double lockTime = 0.0;
   glm::vec3 hammerTip, movedAngle, newPos, displacement, projection;
   CollisionData dat;
   dat = world->checkCollision(this, modelIdx);
   if (dat.hitObj.obj >= 0) {
      printf("Hommur vertex %d node %d hit platform %d face %d at the location (%f, %f, %f) with normal (%f, %f, %f) while moving in the direction (%f, %f, %f) after trying to move (%f, %f, %f)\n",
             /*hammer.model.children[dat.thisObj.nod].name.c_str(), */dat.thisObj.tri, dat.thisObj.mesh, dat.hitObj.obj, dat.hitObj.tri,
             dat.collisionPoint.x, dat.collisionPoint.y,dat.collisionPoint.z,dat.collisionNormal.x, dat.collisionNormal.y,dat.collisionNormal.z,
             dat.collisionAngle.x, dat.collisionAngle.y,dat.collisionAngle.z,dat.collisionStrength.x, dat.collisionStrength.y,dat.collisionStrength.z);
      displacement = dat.collisionAngle-dat.collisionStrength;
      //hammer collides with object
      if (dat.thisObj.mesh == HAMMER_NODE && !pickCollision && !hammerCollision) {
         // rotate back
         setRotation(previousAngle);
         // move back
         //moveBy(displacement);
         moveBy(-getVel()*(float)timeStep);
         setVelocity(-activeForce);
         //moveBy(getVel()*(float)timeStep);
         // give bjorn force
         //m/s         = m/s
         activeForce = dat.collisionStrength * (float)(GRAVITY * 1.5f);
         bjorn->addVelocity(-activeForce);///(float)timeStep);
         printf("Clunk (%f %f %f)\n", activeForce.x, activeForce.y, activeForce.z);
         //                  m          /       s
         // lock rotation temporarily
         lockTime = 0.2;
         locked = true;
         // SMASH
         Sound::hammerSmash();
         hammerCollision = true;
      }
      //pick sank into object
      else if (dat.thisObj.mesh == HAMMER_NODE && pickCollision && !hammerCollision) {
         // move back slightly
         moveBy(displacement*1.2f);
         // set bjorn's velocity
         bjorn->setVelocity(bjorn->getVel() * dat.collisionNormal);
         bjorn->suspend();
         // set collision angle
         pickAngle = glm::normalize(dat.collisionAngle);
         // lock rotation
         locked = true;
         lockTime = 1.0;
         printf("shunk\n");
         hammerCollision = true;
      }
      //pick moved while in object
      else if (dat.thisObj.mesh == HAMMER_NODE && pickCollision && hammerCollision) {
         // rotate back
         setRotation(previousAngle);
         // move back
         moveBy(-getVel()*(float)timeStep);
         // lock rotation
         locked = true;
         lockTime = 1.0;
         printf("crunk\n");
         // move bjorn
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         activeForce = dat.collisionStrength*(float)GRAVITY/2.0f;
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*15.2f));//(float)timeStep);
         bjorn->setVelocity(bjorn->getVel() * dat.collisionNormal);
      }
      //pick hit object
      else if (dat.thisObj.mesh == PICK_NODE && !hammerCollision && !pickCollision) {
         // lock rotation
         lockTime = 1.0;
         pickCollision = true;
         printf("shink\n");
         // set bjorn velocity
         bjorn->setVelocity(bjorn->getVel() * dat.collisionNormal);
         // set hit angle
         pickAngle = glm::normalize(dat.collisionAngle);
         // suspend bjorn
         bjorn->suspend();
      }
      //pick partially in object
      else if (dat.thisObj.mesh == PICK_NODE && !hammerCollision && pickCollision) {
         // lock rotation
         lockTime = 1.0;
         pickCollision = true;
         printf("crank\n");
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         // move back
         moveBy(-getVel()*(float)timeStep);
         // allow movement along insertion angle
         projection = glm::dot(getVel(),pickAngle)*pickAngle;
         moveBy(projection*(float)timeStep);
         // move bjorn
         activeForce = getVel() - projection;
         bjorn->addVelocity(-activeForce);
         bjorn->suspend();
      }
      //pick pulling out of object
      else if (dat.thisObj.mesh == PICK_NODE && hammerCollision && pickCollision) {
         // move back
         moveBy(-getVel()*(float)timeStep);
         // allow movement along insertion angle
         projection = glm::dot(getVel(),pickAngle)*pickAngle;
         moveBy(projection*(float)timeStep);
         // lock rotation
         lockTime = 1.0;
         locked = true;
         printf("unshunk\n");
         hammerCollision = false;
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         activeForce = getVel() - projection;
         bjorn->addVelocity(-activeForce);
         bjorn->suspend();
         setVelocity(bjorn->getVel());
      }
      else if ((dat.thisObj.mesh == PICK_NODE || dat.thisObj.mesh == HAMMER_NODE) && hammerCollision && !pickCollision) {
         //moveBy(displacement);
         moveBy(-getVel()*(float)timeStep);
         bjorn->suspend();
         //m/s         = m/s                     - (m/s^2                * s)
         activeForce = dat.collisionStrength*(float)GRAVITY/4.0f;
         //                  m/s
         if (glm::length(bjornOffset) < 1.79f) {
            bjorn->addVelocity(-activeForce);//(float)timeStep);
         }
         else {
            bjorn->addVelocity((getPos() - bjorn->getPos())*0.05f);
            addVelocity((bjorn->getPos() - getPos())*0.2f);
         }
      }
      //bjorn->suspend();
      movedAngle = getRot();
   }
   else {
      bjorn->unsuspend();
      pickCollision = false;
      hammerCollision = false;
   }
   if (lockTime < 0)
      locked = false;
   else
      lockTime -= timeStep;
}