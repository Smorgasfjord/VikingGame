//
//  Hammer.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/4/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#define GLM_SWIZZLE
#include "Hammer.h"
#define pi 3.14159
#define MAX_ROT 45.0f
#define PICK_TIP1 0
#define PICK_TIP2 2
#define HANDLE_NODE 0
#define WTF_NODE 0
#define WOOD_NODE 3
#define HAMMER_NODE 4
#define PICK_NODE 6
#define STUD_BAND 7
#define STUDS 8
#define GRIP_NODE 9

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
   moveBy(glm::vec3(0, 0, .2));
   scaleBy(glm::vec3(.25f));
   pickNormal = glm::vec3(1.0f,0.0,0.0);
   desiredRotation = previousAngle = glm::vec3(0.0f,180.0,270.0);
   //pick normal = (-1.0, 0, 0)
   rotateBy(desiredRotation);
   pickNormal = (getRotMat() * glm::vec4(pickNormal,0.0f)).xyz();
   this->world = world;
   bjorn = character;
   hammerSide = true;
   hammerCollision = pickCollision = false;
   modelIdx = world->placeObject(this, &simple);
   mountainSide = bjorn->mountainSide;
   bjornOffset = glm::vec3(0.0f);
}

float d2r(float val)
{
   return val * (pi / 180.0f);
}

void Hammer::updatePos(float x, float y)
{
   if (!bjorn->facingRight) x *= -1.0;
   bjornOffset = glm::vec3(bjorn->getRotMat() * glm::vec4(-0.2f,y,x,0.0));
   if (glm::length(bjornOffset) > 1.4) {
      bjornOffset *= 1.4f / glm::length(bjornOffset);
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
            desiredRotation = glm::vec3(desiredRotation.x, desiredRotation.y, -angle * (180.0 / pi));
         else
            desiredRotation = glm::vec3(desiredRotation.x, desiredRotation.y, angle * (180.0 / pi));
      }
      else
      {
         if(((int)getRot().y % 360) == 180)
            desiredRotation = glm::vec3(-angle * (180.0 / pi), desiredRotation.y, desiredRotation.z);
         else
            desiredRotation = glm::vec3(angle * (180.0 / pi), desiredRotation.y, desiredRotation.z);
      }

   }
   if (desiredRotation.x > 180.0) desiredRotation.x -= 360.0;
   else if (desiredRotation.x < -180.0) desiredRotation.x += 360.0;
   if (desiredRotation.y > 180.0) desiredRotation.y -= 360.0;
   else if (desiredRotation.y < -180.0) desiredRotation.y += 360.0;
   if (desiredRotation.z > 180.0) desiredRotation.z -= 360.0;
   else if (desiredRotation.z < -180.0) desiredRotation.z += 360.0;
}


void Hammer::flip()
{
   if (!hammerCollision && !(locked || manualLocked)) {
      hammerSide = !hammerSide;
      rotateBy(glm::vec3(0.0f, 180.0, 0.0));
      desiredRotation += glm::vec3(0.0,180.0f,0.0);
      previousAngle += glm::vec3(0.0,180.0f,0.0);
   }
}

// What hammer does
void Hammer::step(double timeStep)
{
   glm::vec3 rotIncrement, rotFix;
   rotFix = getRot();
   if (rotFix.x > 180.0) rotFix.x -= 360.0;
   else if (rotFix.x < -180.0) rotFix.x += 360.0;
   if (rotFix.y > 180.0) rotFix.y -= 360.0;
   else if (rotFix.y < -180.0) rotFix.y += 360.0;
   if (rotFix.z > 180.0) rotFix.z -= 360.0;
   else if (rotFix.z < -180.0) rotFix.z += 360.0;
   previousAngle = rotFix;
   setRotation(rotFix);
   rotIncrement = (desiredRotation - rotFix) / 2.0f;
   if (glm::length(rotIncrement) > MAX_ROT * 2.0f) {
      rotIncrement = -rotIncrement;
   }
   //printf("desired: (%f,%f,%f), current: (%f,%f,%f), increment: (%f,%f,%f)\n", 
   //      desiredRotation.x,desiredRotation.y,desiredRotation.z, rotFix.x, rotFix.y, rotFix.z, rotIncrement.x, rotIncrement.y, rotIncrement.z);
   if (glm::length(rotIncrement) > MAX_ROT) {
      rotIncrement *= MAX_ROT / glm::length(rotIncrement);
   }
   setVelocity((bjorn->getPos() + bjornOffset - getPos())/((float)timeStep*2.0f));
   if (!pickCollision) {
      addVelocity(bjorn->getVel());
      //setPos(bjorn->getPos());
      
   }
   if (!locked && !manualLocked) {
      rotateBy(rotIncrement);
      pickNormal = glm::vec3(getRotMat() * glm::vec4(1.0,0.0,0.0,0.0f)) * glm::vec3(1.0,-1.0f,1.0);
      printf("pick normal: (%f,%f,%f)\n",pickNormal.x,pickNormal.y,pickNormal.z);
   }
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
             dat.thisObj.tri, dat.thisObj.mesh, dat.hitObj.obj, dat.hitObj.tri,
             dat.collisionPoint.x, dat.collisionPoint.y,dat.collisionPoint.z,dat.collisionNormal.x, dat.collisionNormal.y,dat.collisionNormal.z,
             dat.collisionAngle.x, dat.collisionAngle.y,dat.collisionAngle.z,dat.collisionStrength.x, dat.collisionStrength.y,dat.collisionStrength.z);
       
      displacement = dat.collisionAngle-dat.collisionStrength;
      //hammer collides with object
      if ((dat.thisObj.mesh == HAMMER_NODE || 
               dat.thisObj.mesh == STUDS || dat.thisObj.mesh == STUD_BAND)&& !pickCollision && !hammerCollision) {
         printf("hammer collides with object\n");
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
         //                  m          /       s
         // lock rotation temporarily
         lockTime = 0.2;
         locked = true;
         // SMASH
         Sound::hammerSmash();
         hammerCollision = true;
      }
      //pick sank into object
      else if ((dat.thisObj.mesh != PICK_NODE) && pickCollision && !hammerCollision) {
         printf("pick sank into object\n");
         // move back slightly
         //moveBy(displacement*1.2f);
         moveBy(-getVel()*(float)timeStep);
         setRotation(previousAngle);
         // set bjorn's velocity
         bjorn->setVelocity(bjorn->getVel() * dat.collisionNormal);
         bjorn->suspend();
         // set collision angle
         pickAngle = glm::normalize(dat.collisionAngle);
         // lock rotation
         locked = true;
         lockTime = 1.0;
         hammerCollision = true;
      }
      //pick moved while in object
      else if ((dat.thisObj.mesh != PICK_NODE) && pickCollision && hammerCollision) {
         printf("pick moved while in object\n");
         // rotate back
         setRotation(previousAngle);
         // move back
         moveBy(-getVel()*(float)timeStep);
         // lock rotation
         locked = true;
         lockTime = 1.0;
         // move bjorn
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         projection = glm::dot(getVel(),pickNormal)*pickNormal;
         activeForce = getVel() - projection;
         if (glm::dot(projection,pickNormal) > 0.0) {
            moveBy(projection*(float)timeStep*0.25f);
         }
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*15.2f));//(float)timeStep);
         //bjorn->setVelocity(bjorn->getVel() * dat.collisionNormal);
      }
      //pick hit object
      else if (dat.thisObj.mesh == PICK_NODE && !hammerCollision && !pickCollision && 
            glm::length(pickNormal * glm::normalize(dat.collisionAngle)) > 0.8f) {
         printf("pick hit object\n");
         // lock rotation
         lockTime = 2.0;
         locked = true;
         pickCollision = true;
         // set bjorn velocity
         // set hit angle
         pickAngle = glm::normalize(dat.collisionAngle);
         projection = glm::dot(getVel(),pickNormal)*pickNormal;
         activeForce = (getVel() - projection) * glm::vec3(0,1.0f,0);
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*15.2f));//(float)timeStep);
         // suspend bjorn
         bjorn->suspend();
      }
      //pick partially in object
      else if (dat.thisObj.mesh == PICK_NODE && !hammerCollision && pickCollision) {
         printf("pick partially in object\n");
         // lock rotation
         lockTime = 2.0;
         locked = true;
         pickCollision = true;
         setRotation(previousAngle);
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         // move back
         moveBy(-getVel()*(float)timeStep);
         // allow movement along insertion angle
         projection = glm::dot(getVel(),pickNormal)*pickNormal;
         moveBy(projection*(float)timeStep*0.25f);
         // move bjorn
         activeForce = (getVel() - projection) * glm::vec3(0,1.0f,0);;
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*15.2f));//(float)timeStep);
         bjorn->suspend();
      }
      //pick pulling out of object
      else if (dat.thisObj.mesh == PICK_NODE && hammerCollision && pickCollision) {
         printf("pick pulling out of object\n");
         // move back
         moveBy(-getVel()*(float)timeStep);
         // allow movement along insertion angle
         projection = glm::dot(getVel(),pickNormal)*pickNormal;
         moveBy(projection*(float)timeStep*0.25f);
         setRotation(previousAngle);
         // lock rotation
         lockTime = 2.0;
         locked = true;
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         activeForce = (getVel() - projection) * glm::vec3(0,1.0f,0);;
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*15.2f));//(float)timeStep);
         bjorn->suspend();
      }
      else if (dat.thisObj.mesh != WTF_NODE && dat.thisObj.mesh != WOOD_NODE && dat.thisObj.mesh != GRIP_NODE && !pickCollision) {
         printf("hammer resting against object\n");
         //moveBy(displacement);
         setRotation(previousAngle);
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
      if (glm::length(getVel()) > 0.1) {
      printf("NOOOOO COLLLLLISIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOON\n");
      bjorn->unsuspend();
      pickCollision = false;
      hammerCollision = false;
      }
   }
   if (lockTime < 0)
      locked = false;
   else
      lockTime -= timeStep;
}
