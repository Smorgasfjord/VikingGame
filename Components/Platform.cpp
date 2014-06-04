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
#ifdef _WIN32
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

Platform::Platform(glm::vec3 pos, GLHandles hand, GameModel *model) :
   GameObject("platform")
{
   glm::vec3 rot;
   initialize(*model, 0, 1, hand);
   setPos(Mountain::lockOn(pos,rot));
   setScale(glm::vec3(1.0f, 1.0, 3.0));
   setVelocity(glm::vec3(0));
   mountainSide = Mountain::getSide(pos);
   if(mountainSide == MOUNT_LEFT || mountainSide == MOUNT_RIGHT)
      setRotation(glm::vec3(0, 90, 0));
}

Platform::Platform(glm::vec3 pos, glm::vec3 size, float rotation, int mountSide, GLHandles hand, GameModel *model) :
   GameObject("platform")
{
   glm::vec3 rot;
   mountainSide = mountSide;
   initialize(*model, 0, 1, hand);
   setPos(pos);
   setScale(size);
   setRotation(glm::vec3(0, 0, rotation));
   if(mountSide == MOUNT_LEFT || mountSide == MOUNT_RIGHT)
   {
      rotateBy(glm::vec3(0, 90, 0));
      //I think that rotateBy doesn't work the way we think it does
      if(abs(rotation) > 80 )
         rotateBy(glm::vec3(0, 0, 90));
   }
   setVelocity(glm::vec3(0));
}

string Platform::toString()
{
   char pos[30];
   char sizeStr[30];
   char rot[15];
   char side[15];
   string str = "Platform: \n";
   sprintf(side, "\t%d\n", mountainSide);
   sprintf(pos, "\t%f %f %f\n", getPos().x, getPos().y, getPos().z);
   sprintf(sizeStr, "\t%f %f %f\n", model.state.scale.x, model.state.scale.y, model.state.scale.z);
   sprintf(rot, "\t%f\n", getRot().z);
   str.append(side);
   str.append(pos);
   str.append(sizeStr);
   str.append(rot);
   
   return str;
}

//Read in a .lvl file of the given name
std::vector<Platform> Platform::importLevel(std::string const & fileName, GLHandles handles, GameModel *platMod)
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
      if(getRot().z < 80 && getRot().z > -80)
      {
         if(test.x <= getPos().x + getScale().x && test.x >= getPos().x - getScale().x &&
            test.y <= getPos().y + .5 && test.y >= getPos().y - .5)
            return true;
      }
      else
      {
         if(test.x <= getPos().x + .65 && test.x >= getPos().x - .65 &&
            test.y <= getPos().y + getScale().x && test.y >= getPos().y - getScale().x)
            return true;
      }
   }
   else
   {
      if(test.z <= getPos().z + getScale().x && test.z >= getPos().z - getScale().x &&
         test.y <= getPos().y + .5 && test.y >= getPos().y - .5)
         return true;
   }
   
   return false;
}

void Platform::checkSide()
{
   int curSide = Mountain::getSide(getPos());
   //If the platform has changed sides of the mountain update its rotation
   if(curSide != mountainSide)
   {
      mountainSide = curSide;
      glm::vec3 curRotation = getRot();
      if(mountainSide == MOUNT_FRONT || mountainSide == MOUNT_BACK)
         setRotation(glm::vec3(curRotation.x, 0.0f, curRotation.z));
      else
         setRotation(glm::vec3(curRotation.x, 90.0f, curRotation.z));
   }
}

void Platform::moveDown()
{
   glm::vec3 newPos = getPos(), rot;
   newPos.y -= STEP;
   setPos(Mountain::lockOn(newPos,rot));
   checkSide();
}

void Platform::moveUp()
{
   glm::vec3 newPos = getPos(), rot;
   newPos.y += STEP;
   setPos(Mountain::lockOn(newPos, rot));
   checkSide();
}

void Platform::moveLeft()
{
   glm::vec3 newPos = getPos(), rot;
   if(mountainSide == MOUNT_FRONT)
      newPos.x += STEP;
   else if(mountainSide == MOUNT_BACK)
      newPos.x -= STEP;
   else if(mountainSide == MOUNT_LEFT)
      newPos.z += STEP;
   else
      newPos.z -= STEP;
   setPos(Mountain::lockOn(newPos,rot));
   checkSide();
}

void Platform::moveRight()
{
   glm::vec3 newPos = getPos(), rot;
   if(mountainSide == MOUNT_FRONT)
      newPos.x -= STEP;
   else if(mountainSide == MOUNT_BACK)
      newPos.x += STEP;
   else if(mountainSide == MOUNT_LEFT)
      newPos.z -= STEP;
   else
      newPos.z += STEP;
   setPos(Mountain::lockOn(newPos,rot));
   checkSide();
}

void Platform::step()
{
   return;
}

void Platform::stretch()
{
   scaleBy(glm::vec3(1.05, 1, 1));
}

void Platform::shrink()
{
   scaleBy(glm::vec3(0.95, 1, 1));
}

#endif
