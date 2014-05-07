//
//  Platform.cpp
//  levelBuilder
//
//  Created by Taylor Woods on 4/15/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//
#ifndef PLATFORM_CPP
#define PLATFORM_CPP

#include "Platform.h"
#include "Mountain.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "../Utils/GLSL_helper.h"
#ifdef __WIN32__
#include <time.h>
#endif
#ifdef __APPLE__
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <streambuf>

Platform::Platform()
{
   //This needs to be here for C++ to be happy, even though it doesn't seem to do anything
}

Platform::Platform(glm::vec3 pos, GLHandles hand, Model model)
{
   position = glm::vec3(Mountain::getX(pos), pos.y, Mountain::getZ(pos));
   this->size = glm::vec3(1.0f);
   pGameObject::handles = hand;
   velocity = glm::vec3(0);
   mod = model;
   rotation = 0;
   float fl = Mountain::testLeftDiagonal(pos);
   float fr = Mountain::testRightDiagonal(pos);
   
   //Determine which mountain side we're on
   if(fr > 0 && fl < 0)
      mountainSide = MOUNT_RIGHT;
   else if(fr < 0 && fl > 0)
      mountainSide = MOUNT_LEFT;
   else if(fr > 0 && fl > 0)
      mountainSide = MOUNT_FRONT;
   else
      mountainSide = MOUNT_BACK;
}

Platform::Platform(glm::vec3 pos, glm::vec3 size, float rotation, int mountSide, GLHandles hand, Model model)
{
   position = pos;
   this->size = size;
   this->rotation = rotation;
   mountainSide = mountSide;
   pGameObject::handles = hand;
   velocity = glm::vec3(0);
   mod = model;
}

glm::vec3 Platform::getPos()
{
   return position;
}

string Platform::toString()
{
   char pos[30];
   char sizeStr[30];
   char rot[15];
   char side[15];
   string str = "Platform: \n";
   sprintf(side, "\t%d\n", mountainSide);
   sprintf(pos, "\t%f %f %f\n", position.x, position.y, position.z);
   sprintf(sizeStr, "\t%f %f %f\n", size.x, size.y, size.z);
   sprintf(rot, "\t%f\n", rotation);
   str.append(side);
   str.append(pos);
   str.append(sizeStr);
   str.append(rot);
   
   return str;
}

//Read in a .lvl file of the given name
std::vector<Platform> Platform::importLevel(std::string const & fileName, GLHandles handles, Model platMod)
{
   std::vector<Platform> plats;
   std::ifstream File;
	File.open(fileName.c_str());
   
   if (! File.is_open())
	{
		std::cerr << "Unable to open Level file: " << fileName << std::endl;
	}
   
	while (File)
	{
		string ReadString;
      string pos, size, rot, side;
      glm::vec3 position = glm::vec3(1.0f), sizeVec = glm::vec3(1.0f);
      float rotation;
      int mountainSide;
		getline(File, ReadString);
		std::stringstream Stream(ReadString);
      
      //Platform data
      if(ReadString.find("Platform:") != std::string::npos)
      {
         getline(File, side);
         getline(File, pos);
         getline(File, size);
         getline(File, rot);
         sscanf(side.c_str(), "\t%d\n", &mountainSide);
         sscanf(pos.c_str(), "\t%f %f %f\n", &(position.x), &(position.y), &(position.z));
         sscanf(size.c_str(), "\t%f %f %f\n", &(sizeVec.x), &(sizeVec.y), &(sizeVec.z));
         sscanf(rot.c_str(), "\t%f\n", &rotation);
         plats.push_back(Platform(position, sizeVec, rotation, mountainSide, handles, platMod));
      }
   }
   File.close();
   return plats;
}

//Given a position determines if that position intersects the platform
//Returns false for no collision
bool Platform::detectCollision(glm::vec3 test)
{
   if(mountainSide == MOUNT_FRONT || mountainSide == MOUNT_BACK)
   {
      if(test.x <= position.x + size.x && test.x >= position.x - size.x &&
         test.y <= position.y + .5 && test.y >= position.y - .5)
         return true;
   }
   else
   {
      if(test.z <= position.z + size.x && test.z >= position.z - size.x &&
         test.y <= position.y + .5 && test.y >= position.y - .5)
         return true;
   }
   
   return false;
}

void Platform::moveDown()
{
   position.y -= STEP;
   if(mountainSide == MOUNT_FRONT || mountainSide == MOUNT_BACK)
      position.z = Mountain::getZ(position);
   else
      position.x = Mountain::getX(position);
}

void Platform::moveUp()
{
   position.y += STEP;
   if(mountainSide == MOUNT_FRONT || mountainSide == MOUNT_BACK)
      position.z = Mountain::getZ(position);
   else
      position.x = Mountain::getX(position);
}

void Platform::moveLeft()
{
   if(mountainSide == MOUNT_FRONT)
      position.x += STEP;
   else if(mountainSide == MOUNT_BACK)
      position.x -= STEP;
   else if(mountainSide == MOUNT_LEFT)
      position.z += STEP;
   else
      position.z -= STEP;
}

void Platform::moveRight()
{
   if(mountainSide == MOUNT_FRONT)
      position.x -= STEP;
   else if(mountainSide == MOUNT_BACK)
      position.x += STEP;
   else if(mountainSide == MOUNT_LEFT)
      position.z -= STEP;
   else
      position.z += STEP;
}

float Platform::getRot()
{
   return rotation;
}

void Platform::setRot(float val)
{
   rotation = val;
}

void Platform::step()
{
   return;
}

glm::vec3 Platform::getSize()
{
   return size;
}

void Platform::stretch()
{
   size.x += .05;
}

void Platform::shrink()
{
   size.x -= .05;
}

/* Set up matrices to place model in the world */
void Platform::SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate;
   
   //Spin 90 degrees sideways then apply rotation
   if(mountainSide == MOUNT_LEFT || mountainSide == MOUNT_RIGHT)
      Rotate = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0, 1, 0));
   
   Rotate *= glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
   
   if(mountainSide == MOUNT_FRONT || mountainSide == MOUNT_RIGHT)
      Rotate *= glm::rotate(glm::mat4(1.0f), -rotation / 2, glm::vec3(0, 1, 0));
   else
      Rotate *= glm::rotate(glm::mat4(1.0f), rotation / 2, glm::vec3(0, 1, 0));
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(pGameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(pGameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}

void Platform::draw()
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
#endif
