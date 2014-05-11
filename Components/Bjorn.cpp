//
//  Bjorn.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/3/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#include "Bjorn.h"


#ifdef __APPLE__
#include <sys/time.h>
#endif

#include "Mountain.h"
#include "../Utils/World.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "../Utils/GLSL_helper.h"

#define MAX_SPEED .8
#define HEIGHT 0.5

Bjorn::~Bjorn()
{
}

Bjorn::Bjorn()
{
   
}

Bjorn::Bjorn(glm::vec3 pos, GLHandles hand, Model model, World world)
{
   position = pos;
   position.y += 0.25f;
   position.z += .5f;
   size = glm::vec3(1.0f);
   rotation = 0;
   pGameObject::handles = hand;
   velocity = glm::vec3(0);
   mod = model;
   gravity = -3;
   mass = 20;
   this->world = world;
}


void Bjorn::step()
{
	double curtime = glfwGetTime();
   float deltaT = (float)(curtime -  lastUpdated);
   //Update position based on velocity
   position += deltaT * pGameObject::velocity;
   //Fall due to gravity if not colliding with anything
   if(world.detectCollision(glm::vec3(position.x, position.y - (HEIGHT / 2), position.z)) == 0)
      velocity.y += ((mass * gravity) * .002f);
   else
   {
      position.y = world.getY(position);
      velocity.y = 0;
      //Update X velocity due to friction
      if(velocity.x > 0.1)
         velocity.x -= .05;
      else if (velocity.x < -0.1)
         velocity.x += .05;
      else
         velocity.x = 0;
   }
   
   position.z = Mountain::getZ(position);
   lastUpdated = curtime;
   
   return;
}

glm::vec3 Bjorn::getPos()
{
   return position;
}

/* Set up matrices to place model in the world */
void Bjorn::SetModel(glm::vec3 loc, glm::vec3 size) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0, 1, 0));
   
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(pGameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(pGameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}

void Bjorn::moveRight()
{
   if(velocity.x > -MAX_SPEED)
      velocity.x -= .5;
}

void Bjorn::moveLeft()
{
   if(velocity.x < MAX_SPEED)
      velocity.x += .5f;
}

void Bjorn::jump()
{
   velocity.y = 4;
}

void Bjorn::launch(float angle)
{
   velocity.x -= 2.5 * cos(angle);
   velocity.y += 2.5 * sin(angle);
}

void Bjorn::draw()
{
   //Enable handles
   safe_glEnableVertexAttribArray(handles.aPosition);
   safe_glEnableVertexAttribArray(handles.aNormal);
   
   SetModel(position, size);
   
   glBindBuffer(GL_ARRAY_BUFFER, mod.BuffObj);
   safe_glVertexAttribPointer(handles.aPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
   
   glBindBuffer(GL_ARRAY_BUFFER, mod.NormalBuffObj);
   safe_glVertexAttribPointer(handles.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
   
   glDrawArrays(GL_TRIANGLES, 0, mod.iboLen * 3);
   //clean up
	safe_glDisableVertexAttribArray(handles.aPosition);
	safe_glDisableVertexAttribArray(handles.aNormal);
   return;
}
