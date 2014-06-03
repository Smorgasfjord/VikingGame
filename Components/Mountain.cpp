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
   initialize(*model, 0, 1, hand);
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
   float xfrac, yfrac, xint, yint, depth = 0;

   xfrac = modf(x, &xint);
   yfrac = modf(y, &yint);

   if(x < IMG_MAX_X && x > 0 && y < IMG_MAX_Y && y > IMG_MIN_Y)
   {
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

      norms.x = sin((float)(depths[0] - depths[2])*M_PI/(2.0f*255.0f))*(1.0f-yfrac) + 
                 sin((float)(depths[1] - depths[3])*M_PI/(2.0f*255.0f))*yfrac;
      norms.y = sin((float)(depths[1] - depths[0])*M_PI/(2.0f*255.0f))*(1.0f-xfrac) + 
                 sin((float)(depths[3] - depths[2])*M_PI/(2.0f*255.0f))*xfrac;
      norms.z = sqrt(1.0f - (norms.y*norms.y + norms.x*norms.x));
   }

   norms = glm::normalize(norms);
   return depth;
}

//Round float f to decimal precision pres
float round(float f,int pres)
{
   return floorf(f * pow(10, pres) + 0.5) / pow(10, pres);
}

glm::vec3 Mountain::lockOn(glm::vec3 pos, glm::vec3 & norms)
{
   glm::vec3 roughPos = glm::vec3(round(pos.x, 1), round(pos.y, 1), round(pos.z, 1));
   int side = Mountain::getSide(roughPos);
   float x, y, depthOffset, depth;
   glm::vec3 mountPos = pos;
   
   //map position into texture coordinates, x value changes based on side, y is constant
   if(side == MOUNT_FRONT )
      x = ((float)MOUNT_WIDTH - pos.x) * ((float)IMG_MAX_X / MOUNT_WIDTH);
   else if(side == MOUNT_BACK)
      x = pos.x * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   else if(side == MOUNT_LEFT)
      x = ((float)MOUNT_WIDTH - pos.z) * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   else
      x = pos.z * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   
   y = (((float)MOUNT_HEIGHT - pos.y) * (((float)IMG_MAX_Y - (float)IMG_MIN_Y) / (float)MOUNT_HEIGHT)) + (float)IMG_MIN_Y;
   if (y > 0) { 
      //Index into the data
      depthOffset = interpolateDepth(x,y,side,norms);
      
      //Convert depth back to world
      if(side == MOUNT_FRONT)
         depth = ((float)IMG_MAX_DEPTH - depthOffset) / ((float)IMG_MAX_DEPTH / (float)MOUNT_FRONT_TOP_DEPTH);
      else
         depth = ((float)IMG_MAX_DEPTH - depthOffset) / ((float)IMG_MAX_DEPTH / (float)MOUNT_BACK_TOP_DEPTH);
      //Set the depth based on what side of the mountain the object is on
      if (depthOffset == 0.0f) {
         //Set to 45 degrees, for bad indicies
         norms.x = x - ((float)WIDTH / 2.0f) / ((float)WIDTH);
         norms.z = fabsf((float)WIDTH / 2.0f - x) / ((float)WIDTH);
         norms.y = 0.0f;
         norms = glm::normalize(norms);
      }
      
      if(side == MOUNT_FRONT)
      {
         if(depthOffset > 0.0f) mountPos.z = depth;
      }
      else if(side == MOUNT_BACK) {
         if(depthOffset > 0.0f) mountPos.z = (float)MOUNT_DEPTH - depth;
         norms = (glm::rotate(glm::mat4(1.0f),180.0f,glm::vec3(0.0,1.0f,0.0))*glm::vec4(norms,0.0f)).xyz();
      }
      else if(side == MOUNT_RIGHT) {
         if(depthOffset > 0.0f) mountPos.x = depth;
         norms = (glm::rotate(glm::mat4(1.0f),90.0f,glm::vec3(0.0,1.0f,0.0))*glm::vec4(norms,0.0f)).xyz();
      }
      else {
         if(depthOffset > 0.0f) mountPos.x = (float)MOUNT_WIDTH - depth;
         norms = (glm::rotate(glm::mat4(1.0f),270.0f,glm::vec3(0.0,1.0f,0.0))*glm::vec4(norms,0.0f)).xyz();
      }
      //printf("depth: %f on side %d with normal: (%f, %f, %f) at coords (%f, %f) with position (%f %f %f)\n",depthOffset, side,norms.x,norms.y,norms.z, x, y, roughPos.x, roughPos.y, roughPos.z);
   }
   return mountPos;
}

float Mountain::testLeftDiagonal(glm::vec3 pos)
{
   //Front left diagonal
   //NEW center: <30, 22.25, 30> normal<-1, 0, -1>
   float fld = -(-1.0 * (MOUNT_WIDTH / 2.0)) - (-1.0 * (MOUNT_DEPTH / 2.0));
   return (-1.0 * pos.x) + (-1.0 * pos.z) + fld;
   
}

float Mountain::testRightDiagonal(glm::vec3 pos)
{
   //Front right diagonal
   //NEW center: <30, 22.25, 30> normal:<1, 0, -1>
   float frd = -(1.0 * (MOUNT_WIDTH / 2.0)) - (-1.0 * (MOUNT_DEPTH / 2.0));
   return (1.0 * pos.x) + (-1.0 * pos.z) + frd;
}

int Mountain::getSide(glm::vec3 pos)
{
   int side;
   glm::vec3 norms;
   float fl = Mountain::testLeftDiagonal(pos);
   float fr = Mountain::testRightDiagonal(pos);
   float x,y,z, off;
   off = (float)MOUNT_WIDTH/2.0;
   x = ((float)MOUNT_WIDTH - pos.x) * ((float)IMG_MAX_X / MOUNT_WIDTH);
   y = (((float)MOUNT_HEIGHT - pos.y) * (((float)IMG_MAX_Y - (float)IMG_MIN_Y) / (float)MOUNT_HEIGHT)) + (float)IMG_MIN_Y;
   y = interpolateDepth(x,y,0,norms);
   x = 30.0f;
   z = 30.0f;
   off -= (1.0 - y / 255.0) * off;
   //Determine which mountain side we're on
   if(pos.x < x-off && pos.z < z+off)
      side = MOUNT_LEFT;
   else if(pos.x > x+off && pos.z > z-off)
      side = MOUNT_RIGHT;
   else if(pos.x < x+off && pos.z > z+off)
      side = MOUNT_BACK;
   else
      side = MOUNT_FRONT;
   return side;
}
#endif
