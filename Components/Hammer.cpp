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
#define MAX_FORCE 1.0f
#define PICK_TIP1 0
#define PICK_TIP2 2
#define HANDLE_NODE 0
#define WTF_NODE 4
#define WOOD_NODE 2
#define HAMMER_NODE 1
#define PICK_NODE 3
#define STUD_BAND 5
#define STUDS 6
#define GRIP_NODE 7

float MAX_PICK_DEPTH;

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
   MAX_PICK_DEPTH = simple.contents[PICK_NODE].verts[7].x - simple.contents[PICK_NODE].verts[0].x;
   MAX_PICK_DEPTH /= 3.0;
   initialize(*hammerMod, 0, 0, handles);
   setPos(character->getPos());
   moveBy(glm::vec3(0, 0, .2));
   scaleBy(glm::vec3(.20f));
   pickNormal = glm::vec3(1.0f,0.0,0.0);
   desiredRotation = previousAngle = glm::vec3(0.0f,180.0,270.0);
   setRotation(desiredRotation);
   pickNormal = (getRotMat() * glm::vec4(pickNormal,0.0f)).xyz();
   this->world = world;
   bjorn = character;
   hammerSide = true;
   manualLocked = false;
   hammerCollision = pickCollision = false;
   modelIdx = world->placeObject(this, &simple);
   mountainSide = bjorn->mountainSide;
   bjornOffset = glm::vec3(0.0f);
}

float d2r(float val)
{
   return val * (pi / 180.0f);
}

void Hammer::save() {
   s_state = getState();
   s_mountainSide = mountainSide;
   s_hammerSide = hammerSide;
   s_pickCollision = false;
   s_hammerCollision = false;
   s_locked = false;
   s_desiredRotation = desiredRotation;
   s_previousAngle = previousAngle;
}

void Hammer::reset() {
   setState(s_state);
   mountainSide = s_mountainSide;
   hammerSide = s_hammerSide;
   pickCollision = false;
   hammerCollision = false;
   locked = false;
   desiredRotation = s_desiredRotation;
   previousAngle = s_previousAngle;
}

void Hammer::updatePos(float x, float y)
{
   float xOff, zOff;
   zOff = -0.2*cos((float)mountainSide*M_PI/4.0);
   xOff = 0.2*cos(((float)mountainSide+2.0)*M_PI/4.0);
   if (!bjorn->facingRight) {
      x *= -1.0;
      zOff *= -1.0;
      xOff *= -1.0;
   }
   bjornOffset = glm::vec3(bjorn->getRotMat() * glm::vec4(0.0f,y,x,0.0)) + glm::vec3(xOff,0.0,zOff);
   if (glm::length(bjornOffset) > 1.4) {
      bjornOffset *= 1.4f / glm::length(bjornOffset);
   }
}

void Hammer::updateAngle(float x, float y)
{
   //Vector along which the hammer should end up
   glm::vec3 currentAngle = getRot();
   float direction;
   //Angle between desired vector and neutral hammer position (straight up)
   float angle = atan2(x, y);
   if(!isnan(angle) && !(locked || manualLocked))
   {
      //Save the last angle
      previousAngle = currentAngle;
      direction = hammerSide ? -1.0 : 1.0;
      //Rotation is flipped if the hammer is flipped
      desiredRotation = glm::vec3(desiredRotation.x, desiredRotation.y, direction * angle * (180.0 / pi));
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
   CollisionData dat;
   if (!hammerCollision && !(locked || manualLocked)) {
      rotateBy(glm::vec3(0.0f, 180.0, 0.0));
      dat = world->checkCollision(this, modelIdx);
      if (dat.hitObj.obj < 0) {
         hammerSide = !hammerSide;
         desiredRotation += glm::vec3(0.0,180.0f,0.0);
         previousAngle += glm::vec3(0.0,180.0f,0.0);
      }
      else {
         rotateBy(glm::vec3(0.0f, 180.0, 0.0));
      }
   }
}

// What hammer does
void Hammer::step(double timeStep)
{
   glm::vec3 rotIncrement, rotFix;
   //Update hammer rotation if we're on a different side of the mountain
   if(mountainSide != bjorn->mountainSide)
   {
      if((mountainSide < bjorn->mountainSide || mountainSide == 7 && bjorn->mountainSide == 0) && 
            !(mountainSide == 0 && bjorn->mountainSide == 7))
         desiredRotation.y += 45.0f;
      else
         desiredRotation.y += 315.0f;
      mountainSide = bjorn->mountainSide;
   }
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
      pickNormal = glm::vec3(getRotMat() * glm::vec4(-1.0,0.0, 0.0,0.0f)) * glm::vec3(1.0f,1.0,1.0);
   }
   moveBy(getVel()*(float)timeStep);
   
   return;
}

// How the world reacts to Hommur
void Hammer::update(double timeStep) {
   static double lockTime = 0.0;
   glm::vec3 hammerTip, movedAngle, newPos, displacement, projection, pickMove;
   CollisionData dat;
   dat = world->checkCollision(this, modelIdx);
   if (dat.hitObj.obj >= 0) {
      displacement = dat.collisionAngle-dat.collisionStrength;
      //hammer collides with object
      if ((dat.thisObj.mesh == HAMMER_NODE || 
               dat.thisObj.mesh == STUDS || dat.thisObj.mesh == STUD_BAND)&& !pickCollision) {
         // rotate back
         setRotation(previousAngle);
         // move back
         //moveBy(displacement);
         moveBy(-getVel()*(float)timeStep);
         setVelocity(-activeForce);
         // give bjorn force
         activeForce = dat.collisionStrength * (float)(GRAVITY *0.8);
         bjorn->addVelocity(-activeForce);
         // lock rotation temporarily
         lockTime = 0.1;
         locked = true;
         // SMASH
         if (!hammerCollision) Sound::hammerSmash(max(glm::length(dat.collisionStrength),1.0f));
         hammerCollision = true;
      }
      //pick sank into object
      else if ((dat.thisObj.mesh != PICK_NODE) && pickCollision && !hammerCollision) {
         // move back slightly
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
         // rotate back
         setRotation(previousAngle);
         // move back
         moveBy(-getVel()*(float)timeStep);
         // lock rotation
         locked = true;
         lockTime = 0.5;
         // move bjorn
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         projection = glm::dot(getVel(),pickNormal)*pickNormal;
         activeForce = (getVel() - projection) * glm::vec3(0.1f,(float)GRAVITY,0.1);
         if (glm::length(activeForce) > MAX_FORCE) {
            activeForce *= MAX_FORCE / glm::length(activeForce);
         }
         pickMove = projection*(float)timeStep*0.25f;
         if (glm::dot(projection,pickNormal) > 0.0) {
            if (pickDepth + glm::length(pickMove) > MAX_PICK_DEPTH) {
               moveBy(pickMove*(1.0f-pickDepth/MAX_PICK_DEPTH));
               pickDepth = MAX_PICK_DEPTH;
               bjorn->addVelocity(-pickMove*pickDepth/MAX_PICK_DEPTH);
            }
            else {
               moveBy(pickMove);
               pickDepth += glm::length(pickMove);
               bjorn->addVelocity(-pickMove);
            }
         }
         else {
            moveBy(pickMove);
            pickDepth -= glm::length(pickMove);
            bjorn->addVelocity(-pickMove);
         }
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*5.2f));
      }
      //pick hit object
      else if (dat.thisObj.mesh == PICK_NODE && !hammerCollision && !pickCollision && 
            glm::length(pickNormal * glm::normalize(dat.collisionAngle)) > 0.7f) {
         pickDepth = 0.0;
         // lock rotation
         lockTime = 0.5;
         locked = true;
         pickCollision = true;
         moveBy(-getVel()*(float)timeStep);
         moveBy(dat.collisionAngle);
         // set bjorn velocity
         // set hit angle
         pickAngle = glm::normalize(dat.collisionAngle);
         projection = glm::dot(-displacement,pickNormal)*pickNormal;
         activeForce = (-displacement - projection) * glm::vec3(0,1.0f,0);
         Sound::pickStrike(max(glm::length(dat.collisionStrength),1.0f));
         if (glm::length(activeForce) > MAX_FORCE) {
            activeForce *= MAX_FORCE / glm::length(activeForce);
         }
         pickMove = projection*(float)timeStep*0.25f;
         if (glm::dot(projection,pickNormal) > 0.0) {
            if (pickDepth + glm::length(pickMove) > MAX_PICK_DEPTH) {
               moveBy(pickMove*(1.0f-pickDepth/MAX_PICK_DEPTH));
               pickDepth = MAX_PICK_DEPTH;
               bjorn->addVelocity(-pickMove*pickDepth/MAX_PICK_DEPTH);
            }
            else {
               moveBy(pickMove);
               pickDepth += glm::length(pickMove);
               bjorn->addVelocity(-pickMove);
            }
         }
         else {
            moveBy(pickMove);
            pickDepth -= glm::length(pickMove);
            bjorn->addVelocity(-pickMove);
         }
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*15.2f));//(float)timeStep);
         // suspend bjorn
         bjorn->suspend();
      }
      //pick partially in object
      else if (dat.thisObj.mesh == PICK_NODE && !hammerCollision && pickCollision) {
         // lock rotation
         lockTime = 0.5;
         locked = true;
         pickCollision = true;
         setRotation(previousAngle);
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         // move back
         moveBy(-getVel()*(float)timeStep);
         // allow movement along insertion angle
         projection = glm::dot(getVel(),pickNormal)*pickNormal;
         pickMove = projection*(float)timeStep*0.25f;
         // if pick sinking into object
         if (glm::dot(projection,pickNormal) > 0.0) {
            if (pickDepth + glm::length(pickMove) > MAX_PICK_DEPTH) {
               moveBy(pickMove*(1.0f-pickDepth/MAX_PICK_DEPTH));
               pickDepth = MAX_PICK_DEPTH;
               bjorn->addVelocity(-pickMove*pickDepth/MAX_PICK_DEPTH);
            }
            else {
               moveBy(pickMove);
               pickDepth += glm::length(pickMove);
               bjorn->addVelocity(-pickMove);
            }
         }
         else {
            moveBy(pickMove);
            pickDepth -= glm::length(pickMove);
            bjorn->addVelocity(-pickMove);
         }
         // move bjorn
         activeForce = (getVel() - projection) * glm::vec3(0.1,(float)GRAVITY,0.1);
         if (glm::length(activeForce) > MAX_FORCE) {
            activeForce *= MAX_FORCE / glm::length(activeForce);
         }
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*5.2f));//(float)timeStep);
         bjorn->suspend();
      }
      //pick pulling out of object
      else if (dat.thisObj.mesh == PICK_NODE && hammerCollision && pickCollision) {
         // move back
         moveBy(-getVel()*(float)timeStep);
         // allow movement along insertion angle
         projection = glm::dot(getVel(),pickNormal)*pickNormal;
         pickMove = projection*(float)timeStep*0.25f;
         if (glm::dot(projection,pickNormal) > 0.0) {
            if (pickDepth + glm::length(pickMove) > MAX_PICK_DEPTH) {
               moveBy(pickMove*(1.0f-pickDepth/MAX_PICK_DEPTH));
               pickDepth = MAX_PICK_DEPTH;
               bjorn->addVelocity(-pickMove*pickDepth/MAX_PICK_DEPTH);
            }
            else {
               moveBy(pickMove);
               pickDepth += glm::length(pickMove);
               bjorn->addVelocity(-pickMove);
            }
         }
         else {
            moveBy(pickMove);
            pickDepth -= glm::length(pickMove);
            bjorn->addVelocity(-pickMove);
         }
         setRotation(previousAngle);
         // lock rotation
         lockTime = 1.0;
         locked = true;
         //m/s         = m/s                     * (no unit)           - (m/s^2                * s)
         activeForce = (getVel() - projection) * glm::vec3(0.1,(float)GRAVITY,0.1);
         if (glm::length(activeForce) > MAX_FORCE) {
            activeForce *= MAX_FORCE / glm::length(activeForce);
         }
         bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*5.2f));//(float)timeStep);
         bjorn->suspend();
      }
      else if (dat.thisObj.mesh != HAMMER_NODE && dat.thisObj.mesh != STUDS && dat.thisObj.mesh != STUD_BAND && !pickCollision) {
         setRotation(previousAngle);
         moveBy(-getVel()*(float)timeStep);
         bjorn->suspend();
         //m/s         = m/s                     - (m/s^2                * s)
         activeForce = dat.collisionStrength*(float)GRAVITY/4.0f;
         if (glm::length(activeForce) > MAX_FORCE) {
            activeForce *= MAX_FORCE / glm::length(activeForce);
         }
         //                  m/s
         if (glm::length(bjornOffset) < 1.79f) {
            bjorn->addVelocity(-activeForce/(2.0f+glm::length(bjorn->getVel())*5.2f));//(float)timeStep);
         }
         else {
            bjorn->addVelocity((getPos() - bjorn->getPos())*0.05f);
            addVelocity((bjorn->getPos() - getPos())*0.2f);
         }
      }
   }
   else {
      if (glm::length(getVel()) > 0.1) {
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
