//
//  Hammer.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/4/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#include "Hammer.h"
#define LENGTH 1.0
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
   trans(character->getPos().x - 0.5 - pos().x, character->getPos().y - pos().y, character->getPos().z - pos().z);
   this->world = world;
   bjorn = character;
   rotation = previousAngle = 0.0;
   flipped = 1;
}

float d2r(float val)
{
   return val * (pi / 180.0f);
}

void Hammer::updatePos(float x, float y)
{
   trans(-0.5,0.0,0.0);
}

void Hammer::updateAngle(float x, float y)
{
   glm::vec2 bjornVec = glm::vec2(bjorn->getPos().x, bjorn->getPos().y);
   glm::vec2 desiredHammerTip = glm::vec2(x, y);
   //Vector along which the hammer should end upt
   glm::vec2 desiredToBjorn = glm::normalize(bjornVec - desiredHammerTip);
   //Angle between desired vector and neutral hammer position (straight forward)
   float angle = acos(glm::dot(desiredToBjorn, glm::vec2(-1, 0)));
   if(!isnan(angle))
   {
      //Move hammer so it is centered on bjorn
      trans(cos(d2r(rotation)) * (LENGTH / 2.0f), -sin(d2r(rotation)) * (LENGTH / 2.0f),0.0);
      //Save the last angle
      previousAngle = rotation;
      //Set the rotation
      if(y > bjornVec.y)
         rotation = -angle  * (180.0 / pi);
      //Make sure we aren't hitting a platform, this is pretty fakey
      else if (y <= bjornVec.y  && (angle < (pi / 6.0) || angle > (5.0 * pi / 6.0)))
         rotation = angle  * (180.0 / pi);
      //Hammer is infront of bjorn
      else if (x > bjornVec.x)
         rotation = 30.0;
      //Hammer is behind bjorn
      else
         rotation = 150.0;
      this->rot(0.0,0.0,(flipped*2-1)*d2r(rotation-previousAngle));
      //Move out along new vector
      trans(-cos(d2r(rotation)) * (LENGTH / 2.0f), sin(d2r(rotation)) * (LENGTH / 2.0f),0.0);
   }
   
}


void Hammer::step()
{
   glm::vec3 position;
   position.y = bjorn->getPos().y + (sin(d2r(-rotation)) * (LENGTH / 2.0f));
   position.x = bjorn->getPos().x - (cos(d2r(-rotation)) * (LENGTH / 2.0f));
   position.z = Mountain::getZ(position);
   trans(position.x - pos().x, position.y - pos().y, position.z - pos().z);
   glm::vec2 hammerTip = glm::vec2(position.x, position.y);
   hammerTip.y += sin(d2r(-rotation)) * (LENGTH / 2.0f);
   hammerTip.x -= cos(d2r(-rotation)) * (LENGTH / 2.0f);
   if(world.detectCollision(glm::vec3(hammerTip.x, hammerTip.y, position.z)) == 1 && abs(previousAngle - rotation) > 4)
   {
      bjorn->launch(45);
      collision = true;
   }
   else
      collision = false;
   return;
}

