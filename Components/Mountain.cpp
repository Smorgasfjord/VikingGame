//
//  Mountain.cpp
//  levelBuilder
//
//  Created by Taylor Woods on 4/16/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//
#ifndef MOUNTAIN_CPP
#define MOUNTAIN_CPP

#include "Mountain.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "../Utils/GLSL_helper.h"
#ifdef _WIN32
#include <time.h>
#endif
#ifdef __APPLE__
#include <sys/time.h>
#endif

Mountain::Mountain()
{
   
}

Mountain::Mountain(glm::vec3 pos, GLHandles hand, Model model)
{
   position = pos;
   size = glm::vec3(1.0f);
   rotation = 0;
   pGameObject::handles = hand;
   velocity = glm::vec3(0);
   mod = model;
}

void Mountain::step()
{
   return;
}

/* Set up matrices to place model in the world */
void Mountain::SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(pGameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(pGameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}


float Mountain::testLeftDiagonal(glm::vec3 pos)
{
   //Front left diagonal
   //center:<30, 15, 30> normal:<1, -1, -.5>
   float fld = -(1 * 30) - (-1 * 15) - (-.5 * 30);
   return (1 * pos.x) + (-1 * pos.y) + (-.5 * pos.z) + fld;
}

float Mountain::testRightDiagonal(glm::vec3 pos)
{
   //Front right diagonal
   //center:<30, 15, 30> normal<-1, -1, -.5>
   float frd = -(-1 * 30) - (-1 * 15) - (-.5 * 30);
   return (-1 * pos.x) + (-1 * pos.y) + (-.5 * pos.z) + frd;
}

int Mountain::getSide(glm::vec3 pos)
{
   int side;
   float fl = Mountain::testLeftDiagonal(pos);
   float fr = Mountain::testRightDiagonal(pos);
   
   
   //Determine which mountain side we're on
   if(fr > 0 && fl < 0)
      side = MOUNT_RIGHT;
   else if(fr < 0 && fl > 0)
      side = MOUNT_LEFT;
   else if(fr > 0 && fl > 0)
      side = MOUNT_FRONT;
   else
      side = MOUNT_BACK;
   return side;
}

//Given a vector position determine the proper x
float Mountain::getX(glm::vec3 pos)
{
   //ax + by + cz + d = 0 -> x = (-by - cz -d) / a
   //d = - ax0 -by0 - cz0
   float x;
   float fl = testLeftDiagonal(pos);
   float fr = testRightDiagonal(pos);
   cout << "Front left: " << fl << " Front Right: " << fr << "\n";
   //Right face
   if(fr > 0 && fl < 0)
   {
      //cout << "RIGHT\n";
      //center:<15, 15, 30> normal<-1, 1, 0>
      float d = -(-1 * 15) - (1 * 15);
      x = (-(1 * pos.y) - d) / -1;
   }
   //Left face
   else if(fr < 0 && fl > 0)
   {
      //cout << "LEFT\n";
      //center:<45, 15, 30> normal <1, 1, 0>
      float d = -(1 * 45) - (1 * 15);
      x = (-(1 * pos.y) - d) / 1;
   }
   else
      x = pos.x;
   
   return x;
}

//Given a vector position determine the proper z
float Mountain::getZ(glm::vec3 pos)
{
   //ax + by + cz + d = 0 -> z = (-ax - by -d) / c
   //d = - ax0 -by0 - cz0
   float z;
   float fl = testLeftDiagonal(pos);
   float fr = testRightDiagonal(pos);
   cout << "Front left: " << fl << " Front Right: " << fr << "\n";
   //Front face
   if(fr > 0 && fl > 0)
   {
      //center:<30, 15, 15> normal<0, 1, -1>
      float d = -(1 * 15) - (-1 * 15);
      z = (-(1 * pos.y) - d) / -1;
   }
   //Back face
   else if(fr < 0 && fl < 0)
   {
      //center:<30, 15, 45> normal:<0, 1, 1>
      float d = -(1 * 15) - (1 * 45);
      z = (-(1 * pos.y) - d) / 1;
   }
   else
      z = pos.z;
   
   return z;
}

void Mountain::draw()
{
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
#endif
