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
Hammer::Hammer(std::string n)
   :GameObject(n)
{
   
}

void Hammer::setInWorld(World world, Bjorn *character)
{
   setPos(character->getPos());
   scaleBy(glm::vec3(.3f));
   rotateBy(glm::vec3(0, 180, -90));
   this->world = world;
   bjorn = character;
   previousAngle = 0.0;
   flipped = 1;
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
   glm::vec2 desiredHammerTip = glm::vec2(x, y);
   //Vector along which the hammer should end upt
   glm::vec2 desiredToBjorn = glm::normalize(bjornVec - desiredHammerTip);
   glm::vec3 currentAngle = getRot();
   //Angle between desired vector and neutral hammer position (straight forward)
   float angle = acos(glm::dot(desiredToBjorn, glm::vec2(-1, 0)));
   if(!isnan(angle))
   {
      //Move hammer so it is centered on bjorn
      //translateBy(cos(d2r(rotation)) * (LENGTH / 2.0f), -sin(d2r(rotation)) * (LENGTH / 2.0f),0.0);
      //Save the last angle
      previousAngle = currentAngle.z;
      //Set the rotation theres some weird offsets in here but i don't care enough
      if(y > bjornVec.y)
         setRotation(glm::vec3(currentAngle.x, currentAngle.y, angle  * (180.0 / pi) - 90));
      //Make sure we aren't hitting a platform, this is pretty fakey
      else if (y <= bjornVec.y  && (angle < (pi / 6.0) || angle > (5.0 * pi / 6.0)))
         setRotation(glm::vec3(currentAngle.x, currentAngle.y, -angle  * (180.0 / pi) - 90));
      //Hammer is infront of bjorn
      else if (x > bjornVec.x)
         setRotation(glm::vec3(currentAngle.x, currentAngle.y, -120));
      //Hammer is behind bjorn
      else
         setRotation(glm::vec3(currentAngle.x, currentAngle.y, 120));
   }
   
}


void Hammer::step()
{
   setPos(bjorn->getPos());
   glm::vec3 hammerTip = getPos();
   hammerTip.y += sin(d2r(getRot().z + 90));
   hammerTip.x -= cos(d2r(getRot().z + 90));
   if(world.detectCollision(hammerTip) == 1 && abs(previousAngle - getRot().z) > 4)
   {
      cout << "COllision\n";
      bjorn->launch(45);
      collision = true;
   }
   else
      collision = false;
   return;
}

