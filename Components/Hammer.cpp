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

void Hammer::setInWorld(World world, Bjorn *character)
{
   setPos(character->getPos());
   moveBy(glm::vec3(0, .5, .25));
   scaleBy(glm::vec3(.3f));
   rotateBy(glm::vec3(0, 180, -90));
   this->world = world;
   bjorn = character;
   previousAngle = 0.0;
   hammerSide = true;
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
   if(!isnan(angle))
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


void Hammer::step()
{
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
   
   glm::vec3 hammerTip = getPos();
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
         Sound::hammerSmash();
      }
      else if(!hammerSide)
         bjorn->suspend();
      collision = true;
   }
   else
   {
      collision = false;
      bjorn->unsuspend();
   }
   return;
}

