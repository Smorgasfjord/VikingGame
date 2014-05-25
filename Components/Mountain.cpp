//
//  Mountain.cpp
//  levelBuilder
//
//  Created by Taylor Woods on 4/16/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//
#ifndef MOUNTAIN_CPP
#define MOUNTAIN_CPP

#define GLM_SWIZZLE
#include "Mountain.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "../Utils/GLSL_helper.h"
// include DevIL for image loading
#include <IL/il.h>
#ifdef _WIN32
#include <time.h>
#endif
#ifdef __APPLE__
#include <sys/time.h>
#endif

#define WIDTH 512
#define MOUNT_FRONT_TOP_DEPTH 28
#define MOUNT_BACK_TOP_DEPTH 28
#define MOUNT_WIDTH 60
#define MOUNT_DEPTH 60
#define MOUNT_HEIGHT 44.5
#define IMG_MAX_X 511
#define IMG_MAX_Y 507
#define IMG_MIN_Y 130
#define IMG_MAX_DEPTH 223
#define OFFSET_BASE 20.5
#define OFFSET_EXPONENT 8

//Top of mountain (30, 44.5, 30)
//Far left (60, 0, 0)
//Far back (0, 0, 60)
ILubyte* heightMaps[4]; //pointers to heightmaps

Mountain::Mountain()
{
   
}

Mountain::Mountain(GLHandles hand, GameModel *model)
{
   initialize(model, 0, 1, hand);
   setScale(glm::vec3(1.0f));
   setPos(glm::vec3(30, 22.5, 30));
   
   loadHeightMaps();
}

void Mountain::loadHeightMaps()
{
   ILboolean success;
   std::string filename;
   /* create and fill array with DevIL texture ids */
   ILuint* imageIds = new ILuint[4];
   ilGenImages(4, imageIds);
   
   for(int i = 0; i < 4; i++)
   {
      //save IL image ID
      //Front and back are reversed
      if(i == MOUNT_FRONT)
         filename = "Models/mountain_depthmap_back.jpg";
      else if(i == MOUNT_BACK)
         filename = "Models/mountain_depthmap_front.jpg";
      else if(i == MOUNT_LEFT)
         filename = "Models/mountain_depthmap_right.jpg";
      else
         filename = "Models/mountain_depthmap_left.jpg";
      
      ilBindImage(imageIds[0]); /* Binding of DevIL image name */
      ilEnable(IL_ORIGIN_SET);
      ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
      success = ilLoadImage((ILstring)filename.c_str());
      
      if (success) {
         /* Convert image to RGBA */
         ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
         
         cout << "Loaded height map " << filename << "\n";
         heightMaps[i] = ilGetData();
         
      }
      else
         printf("Couldn't load Image: %s\n", filename.c_str());
   }
}

float interpolateDepth(float x, float y, int side, glm::vec3 & norms) {
   int depths[4];
   float interps[4];
   float xfrac, yfrac, xint, yint, depth;

   xfrac = modf(x, &xint);
   yfrac = modf(y, &yint);

   depths[0] = heightMaps[side][((int)y * WIDTH + (int)x) * 4];
   depths[1] = heightMaps[side][((int)(y+1) * WIDTH + (int)x) * 4];
   depths[2] = heightMaps[side][((int)y * WIDTH + (int)(x+1)) * 4];
   depths[3] = heightMaps[side][((int)(y+1) * WIDTH + (int)(x+1)) * 4];

   interps[0] = (float)depths[0] * (1.0f - xfrac) * (1.0f - yfrac); 
   interps[1] = (float)depths[1] * (1.0f - xfrac) * yfrac;
   interps[2] = (float)depths[2] * xfrac * (1.0f - yfrac);
   interps[3] = (float)depths[3] * xfrac * yfrac;
   depth = interps[0];
   depth += interps[1];
   depth += interps[2];
   depth += interps[3];

   norms.x = sin((float)(depths[0] - depths[2])*M_PI/(2.0f*(float)IMG_MAX_DEPTH))*(1.0f-yfrac) + 
              sin((float)(depths[1] - depths[3])*M_PI/(2.0f*(float)IMG_MAX_DEPTH))*yfrac;
   norms.y = sin((float)(depths[1] - depths[0])*M_PI/(2.0f*(float)IMG_MAX_DEPTH))*(1.0f-xfrac) + 
              sin((float)(depths[3] - depths[2])*M_PI/(2.0f*(float)IMG_MAX_DEPTH))*xfrac;
   norms.z = sqrt(1.0f - (norms.y*norms.y + norms.x*norms.x));

   return depth;
}

glm::vec3 Mountain::lockOn(glm::vec3 pos, glm::vec3 & norms)
{
   int side = Mountain::getSide(pos);
   int x,y;
   float depthOffset;
   float depth, testDepth, testx, testy;
   glm::vec3 mountPos = pos;
   
   //map position into texture coordinates, x value changes based on side, y is constant
   if(side == MOUNT_FRONT ) {
      x = (MOUNT_WIDTH - pos.x) * (IMG_MAX_X / MOUNT_WIDTH);
      testx = ((float)MOUNT_WIDTH - pos.x) * ((float)IMG_MAX_X / MOUNT_WIDTH);
   }
   else if(side == MOUNT_BACK) {
      x = pos.x * (IMG_MAX_X / MOUNT_WIDTH);
      testx = pos.x * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   }
   else if(side == MOUNT_LEFT) {
      x = (MOUNT_WIDTH - pos.z) * (IMG_MAX_X / MOUNT_WIDTH);
      testx = ((float)MOUNT_WIDTH - pos.z) * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   }
   else {
      x = pos.z * (IMG_MAX_X / MOUNT_WIDTH);
      testx = pos.z * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   }
   
   y = ((MOUNT_HEIGHT - pos.y) * ((IMG_MAX_Y - IMG_MIN_Y) / MOUNT_HEIGHT)) + IMG_MIN_Y;
   testy = (((float)MOUNT_HEIGHT - pos.y) * (((float)IMG_MAX_Y - (float)IMG_MIN_Y) / (float)MOUNT_HEIGHT)) + (float)IMG_MIN_Y;
   if (y > 0) { 
      //Index into the data
      depthOffset = interpolateDepth(testx,testy,side,norms);
      printf("Bjorn depth: %f with normal: (%f, %f, %f)\n",depthOffset,norms.x,norms.y,norms.z);
      
      //Convert depth back to world
      if(side == MOUNT_FRONT)
         depth = ((float)IMG_MAX_DEPTH - depthOffset) / ((float)IMG_MAX_DEPTH / (float)MOUNT_FRONT_TOP_DEPTH);
      else
         depth = ((float)IMG_MAX_DEPTH - depthOffset) / ((float)IMG_MAX_DEPTH / (float)MOUNT_BACK_TOP_DEPTH);
      //Set the depth based on what side of the mountain the object is on
      if(side == MOUNT_FRONT)
         mountPos.z = depth;
      else if(side == MOUNT_BACK) {
         mountPos.z = (float)MOUNT_DEPTH - depth;
         norms = (glm::rotate(glm::mat4(1.0f),180.0f,glm::vec3(0.0,1.0f,0.0))*glm::vec4(norms,0.0f)).xyz();
      }
      else if(side == MOUNT_RIGHT) {
         mountPos.x = depth;
         norms = (glm::rotate(glm::mat4(1.0f),90.0f,glm::vec3(0.0,1.0f,0.0))*glm::vec4(norms,0.0f)).xyz();
      }
      else {
         mountPos.x = (float)MOUNT_WIDTH - depth;
         norms = (glm::rotate(glm::mat4(1.0f),270.0f,glm::vec3(0.0,1.0f,0.0))*glm::vec4(norms,0.0f)).xyz();
      }
   }
   return mountPos;
}

float Mountain::testLeftDiagonal(glm::vec3 pos)
{
   //Front left diagonal
   //NEW center: <30, 22.25, 30> normal<-1, 0, -1>
   float fld = -(-1 * (MOUNT_WIDTH / 2)) - (-1 * (MOUNT_DEPTH / 2));
   return (-1 * pos.x) + (-1 * pos.z) + fld;
   
}

float Mountain::testRightDiagonal(glm::vec3 pos)
{
   //Front right diagonal
   //NEW center: <30, 22.25, 30> normal:<1, 0, -1>
   float frd = -(1 * (MOUNT_WIDTH / 2)) - (-1 * (MOUNT_DEPTH / 2));
   return (1 * pos.x) + (-1 * pos.z) + frd;
}

int Mountain::getSide(glm::vec3 pos)
{
   int side;
   float fl = Mountain::testLeftDiagonal(pos);
   float fr = Mountain::testRightDiagonal(pos);
   //cout << "Front left: " << fl << " Front Right: " << fr << "\n";
   
   //Determine which mountain side we're on
   if(fr > 0 && fl < 0)
      side = MOUNT_LEFT;
   else if(fr < 0 && fl > 0)
      side = MOUNT_RIGHT;
   else if(fr > 0 && fl > 0)
      side = MOUNT_FRONT;
   else
      side = MOUNT_BACK;
   return side;
}
/*
//Given a vector position determine the proper x
float Mountain::getX(glm::vec3 pos)
{
   //ax + by + cz + d = 0 -> x = (-by - cz -d) / a
   //d = - ax0 -by0 - cz0
   float x;
   float fl = testLeftDiagonal(pos);
   float fr = testRightDiagonal(pos);
   
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
*/
#endif
