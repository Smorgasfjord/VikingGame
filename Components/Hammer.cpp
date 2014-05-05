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
   rotation = 0;
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

void Hammer::updateAngle(float dx, float dy)
{
   glm::vec2 hammerTip = glm::vec2(position.x, position.y);
   hammerTip.y += sin(d2r(-rotation)) * (LENGTH / 2.0f);
   hammerTip.x -= cos(d2r(-rotation)) * (LENGTH / 2.0f);
   glm::vec2 bjornVec = glm::vec2(bjorn->getPos().x, bjorn->getPos().y);
   glm::vec2 desiredHammerTip = glm::vec2(hammerTip.x - dx, hammerTip.y + dy);
   glm::vec2 desiredToBjorn = bjornVec - desiredHammerTip;
   glm::vec2 currentToBjorn = bjornVec - hammerTip;
   float dotP = glm::dot(desiredToBjorn, currentToBjorn);
   float angle = acos( dotP / (currentToBjorn.length() * desiredToBjorn.length()));
   if(!isnan(angle))
   {
      cout << "Current Rotation: " << rotation << "\n";
      cout << "\tdx/dy: (" << dx << ", " << dy << ")\n";
      cout << "\tHammer Position: (" << position.x << ", " << position.y << ")\n";
      cout << "\tHammer Tip: (" << hammerTip.x << ", " << hammerTip.y << ")\n";
      cout << "\tBjorn: (" << bjornVec.x << ", " << bjornVec.y << ")\n";
      cout << "\tDesired Hammer Tip: (" << desiredHammerTip.x << ", " << desiredHammerTip.y << ")\n";
      cout << "\tDesired to Bjorn: (" << desiredToBjorn.x << ", " << desiredToBjorn.y << ")\n";
      cout << "\tCurrent to Bjorn: (" << currentToBjorn.x << ", " << currentToBjorn.y << ")\n";
      cout << "\tDotP: " << dotP << "\n";
      cout << "\tangle " << angle * (180 / pi) << "\n";
      //Move hammer so it is centered on bjorn
      position.y -= sin(d2r(-rotation)) * (LENGTH / 2.0f);
      position.x += cos(d2r(-rotation)) * (LENGTH / 2.0f);
      //Rotate by angle
      if(angle < 180)
         rotation += angle * (180 / pi);
      else
         rotation -= angle * (180 / pi);
      if (rotation > 180) {
         rotation -= 360;
      }
      //Move out along new vector
      position.y += sin(d2r(-rotation)) * (LENGTH / 2.0f);
      position.x -= cos(d2r(-rotation)) * (LENGTH / 2.0f);
   }
   
}

/* Set up matrices to place model in the world */
void Hammer::SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
   
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(GameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(GameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}

void Hammer::step()
{
   position.y = bjorn->getPos().y + (sin(d2r(-rotation)) * (LENGTH / 2.0f));
   position.x = bjorn->getPos().x - (cos(d2r(-rotation)) * (LENGTH / 2.0f));
   position.z = Mountain::getZ(position);
   glm::vec2 hammerTip = glm::vec2(position.x, position.y);
   hammerTip.y += sin(d2r(-rotation)) * (LENGTH / 2.0f);
   hammerTip.x -= cos(d2r(-rotation)) * (LENGTH / 2.0f);
   if(world.detectCollision(glm::vec3(hammerTip.x, hammerTip.y, position.z)) == 1 && !collision)
   {
      cout << "Hit a platform";
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

