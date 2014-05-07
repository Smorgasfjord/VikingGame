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
Hammer::Hammer()
{
   
}

Hammer::Hammer(GLHandles hand, Model model, World world, Bjorn *character)
{
   position = character->getPos();
   position.x -= .5;
   handles = hand;
   size = glm::vec3(1.0f);
   mod = model;
   this->world = world;
   bjorn = character;
   rotation = previousAngle = 0;
}

float d2r(float val)
{
   return val * (pi / 180.0f);
}

void Hammer::updatePos(float x, float y)
{
   position.x = x - .5;
   position.y = y;
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
      position.y -= sin(d2r(rotation)) * (LENGTH / 2.0f);
      position.x += cos(d2r(rotation)) * (LENGTH / 2.0f);
      //Save the last angle
      previousAngle = rotation;
      //Set the rotation
      if(y > bjornVec.y)
         rotation = -angle  * (180 / pi);
      //Make sure we aren't hitting a platform, this is pretty fakey
      else if (y <= bjornVec.y  && (angle < (pi / 6) || angle > (5 * pi / 6)))
         rotation = angle  * (180 / pi);
      //Hammer is infront of bjorn
      else if (x > bjornVec.x)
         rotation = 30;
      //Hammer is behind bjorn
      else
         rotation = 150;

      //Move out along new vector
      position.y += sin(d2r(rotation)) * (LENGTH / 2.0f);
      position.x -= cos(d2r(rotation)) * (LENGTH / 2.0f);
   }
   
}

/* Set up matrices to place model in the world */
void Hammer::SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
   
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(pGameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(pGameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}

void Hammer::step()
{
   position.y = bjorn->getPos().y + (sin(d2r(-rotation)) * (LENGTH / 2.0f));
   position.x = bjorn->getPos().x - (cos(d2r(-rotation)) * (LENGTH / 2.0f));
   position.z = Mountain::getZ(position);
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

void Hammer::draw()
{
   //Enable handles
   safe_glEnableVertexAttribArray(handles.aPosition);
   safe_glEnableVertexAttribArray(handles.aNormal);
   
   SetModel(position, size, rotation);
   
   glBindBuffer(GL_ARRAY_BUFFER, mod.BuffObj);
   safe_glVertexAttribPointer(handles.aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
   
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mod.IndxBuffObj);
   safe_glEnableVertexAttribArray(handles.aNormal);
   
   glBindBuffer(GL_ARRAY_BUFFER, mod.NormalBuffObj);
   safe_glVertexAttribPointer(handles.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
   
   glDrawElements(GL_TRIANGLES, mod.iboLen, GL_UNSIGNED_SHORT, 0);
   //clean up
	safe_glDisableVertexAttribArray(handles.aPosition);
	safe_glDisableVertexAttribArray(handles.aNormal);
   return;
}

