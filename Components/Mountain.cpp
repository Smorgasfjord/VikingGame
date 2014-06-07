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
#define MOUNT_WIDTH 60.0
#define MOUNT_DEPTH 60.0
#define MOUNT_HEIGHT 50.0

#define IMG_MAX_X 511
#define IMG_MAX_Y 507
#define IMG_MIN_Y 130
#define IMG_MAX_DEPTH 223
#define OFFSET_BASE 20.5
#define OFFSET_EXPONENT 8

//Top of mountain (30, 44.5, 30)
//Far left (60, 0, 0)
//Far front (0, 0, 60)
ILubyte* heightMaps[4]; //pointers to heightmaps
static Plane_t front;
static Plane_t fr;
static Plane_t rght;
static Plane_t rb;
static Plane_t back;
static Plane_t bl;
static Plane_t lft;
static Plane_t lf;

Mountain::Mountain()
{
   
}

Mountain::Mountain(GLHandles hand, GameModel *model)
{
   initialize(*model, 0, 1, hand);
   setScale(glm::vec3(1.0f));
   setPos(glm::vec3(30, 22.5, 30));
   genPlanes();
   //loadHeightMaps();
}

void Mountain::genPlanes() {
   front.c = -1.0;
   front.a = 0.0;
   front.b = MOUNT_WIDTH/2.0 / MOUNT_HEIGHT;
   front.d = 0.0;

   rght.b = MOUNT_DEPTH/2.0 / MOUNT_HEIGHT;
   rght.c = 0.0;
   rght.a = -1.0;
   rght.d = 0.0;

   back.a = 0.0;
   back.b = MOUNT_WIDTH/2.0 / MOUNT_HEIGHT;
   back.c = 1.0;
   back.d = -MOUNT_DEPTH;

   lft.a = 1.0;
   lft.b = MOUNT_DEPTH/2.0 / MOUNT_HEIGHT;
   lft.c = 0.0;
   lft.d = -MOUNT_WIDTH;
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
      //Front and front are reversed
      if(i == MOUNT_FRONT)
         filename = "Models/mountain_depthmap_front.jpg";
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

float interpolateDepth(float x, float y, int sideOne, int sideTwo, glm::vec3 & norms) {
   int depths[8];
   float interps[8];
   float xfrac, yfrac, xint, yint, depthOne = 0, depthTwo = 0, depth;

   xfrac = modf(x, &xint);
   yfrac = modf(y, &yint);

   if(x < IMG_MAX_X && x > 0 && y < IMG_MAX_Y && y > IMG_MIN_Y)
   {
      depths[0] = heightMaps[sideOne][((int)y * WIDTH + (int)x) * 4];
      depths[1] = heightMaps[sideOne][((int)(y+1) * WIDTH + (int)x) * 4];
      depths[2] = heightMaps[sideOne][((int)y * WIDTH + (int)(x+1)) * 4];
      depths[3] = heightMaps[sideOne][((int)(y+1) * WIDTH + (int)(x+1)) * 4];
      depths[4] = heightMaps[sideTwo][((int)y * WIDTH + WIDTH-(int)x) * 4];
      depths[5] = heightMaps[sideTwo][((int)(y+1) * WIDTH + WIDTH-(int)x) * 4];
      depths[6] = heightMaps[sideTwo][((int)y * WIDTH + WIDTH-(int)(x+1)) * 4];
      depths[7] = heightMaps[sideTwo][((int)(y+1) * WIDTH + WIDTH-(int)(x+1)) * 4];

      interps[0] = (float)depths[0] * (1.0f - xfrac) * (1.0f - yfrac); 
      interps[1] = (float)depths[1] * (1.0f - xfrac) * yfrac;
      interps[2] = (float)depths[2] * xfrac * (1.0f - yfrac);
      interps[3] = (float)depths[3] * xfrac * yfrac;
      interps[6] = (float)depths[6] * (1.0f - xfrac) * (1.0f - yfrac); 
      interps[7] = (float)depths[7] * (1.0f - xfrac) * yfrac;
      interps[4] = (float)depths[4] * xfrac * (1.0f - yfrac);
      interps[5] = (float)depths[5] * xfrac * yfrac;
      depthOne = interps[0] + interps[1] + interps[2] + interps[3];
      depthTwo = sideOne == sideTwo ? depthOne : interps[4] + interps[5] + interps[6] + interps[7];

      depth = sideOne == sideTwo ? depthOne : sqrt(depthOne*depthOne + depthTwo*depthTwo);

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
   int side = Mountain::getSide(pos);
   float x, y,depthOffset, depthOffsetX, depthOffsetZ, depthX, depthZ, angle;
   glm::vec3 mountPos = pos;
   
   //map position into texture coordinates, x value changes based on side, y is constant
   /*if(side == MOUNT_FRONT || side == MOUNT_FR )
      x = ((float)MOUNT_WIDTH - pos.x) * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   else if(side == MOUNT_BACK || side == MOUNT_BL)
      x = pos.x * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   else if(side == MOUNT_LEFT || side == MOUNT_LF)
      x = ((float)MOUNT_WIDTH - pos.z) * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);
   else
      x = pos.z * ((float)IMG_MAX_X / (float)MOUNT_WIDTH);*/
   
   norms = glm::normalize(glm::vec3(((float)MOUNT_WIDTH/2.0)-pos.x,0.0f,((float)MOUNT_WIDTH/2.0)-pos.z));
   //y = (((float)MOUNT_HEIGHT - pos.y) * (((float)IMG_MAX_Y - (float)IMG_MIN_Y) / (float)MOUNT_HEIGHT)) + (float)IMG_MIN_Y;
   //if (y > 0 && x > 0) { 
      //Index into the data
      /*if (side % 2 == 0)
         depthOffset = interpolateDepth(x,y,side/2,side/2,norms);
      else {
         depthOffset = interpolateDepth(x,y,(side+7)%8/2,(side+1)%8/2,norms);
         norms = glm::normalize(glm::vec3(((float)MOUNT_WIDTH/2.0)-pos.x,0.0f,((float)MOUNT_WIDTH/2.0)-pos.z));
      }*/
      //Convert depth front to world
      /*if(side == MOUNT_FRONT)
         depth = ((float)IMG_MAX_DEPTH - depthOffset) / ((float)IMG_MAX_DEPTH / (float)MOUNT_FRONT_TOP_DEPTH);
      else
         depth = ((float)IMG_MAX_DEPTH - depthOffset) / ((float)IMG_MAX_DEPTH / (float)MOUNT_BACK_TOP_DEPTH);
      *///Set the depth based on what side of the mountain the object is on
      /*if (depthOffset == 0.0f) {
         //Set to 45 degrees, for bad indicies
         norms.x = x - ((float)WIDTH / 2.0f) / ((float)WIDTH);
         norms.z = fabsf((float)WIDTH / 2.0f - x) / ((float)WIDTH);
         norms.y = 0.0f;
         norms = glm::normalize(norms);
      }*/
      
      switch (side) {
      case MOUNT_FRONT:
         mountPos.z = (front.a*pos.x + front.b*pos.y + front.d) / -front.c - 1.0f;
         norms = glm::vec3(0,0,1.0f);
         break;
      case MOUNT_FR:
         depthZ = (front.a*pos.x + front.b*pos.y + front.d) / front.c;
         depthX = (rght.b*pos.y + rght.c*pos.z + rght.d) / rght.a;
         depthOffsetZ = fabsf(fabsf(depthZ) - pos.z);
         depthOffsetX = fabsf(fabsf(depthX) - pos.x);
         depthOffset = sqrt(depthOffsetX*depthOffsetX + depthOffsetZ*depthOffsetZ);
         norms = glm::normalize(glm::vec3(depthOffsetX,0.0f,depthOffsetZ));
         mountPos.z = fabsf(depthZ) - depthOffsetZ/depthOffset;
         mountPos.x = fabsf(depthX) - depthOffsetX/depthOffset;
         break;
      case MOUNT_RIGHT:
         mountPos.x = (rght.b*pos.y + rght.c*pos.z + rght.d) / -rght.a - 1.0f;
         norms = glm::vec3(1.0f,0,0);
         break;
      case MOUNT_RB:
         depthZ = (back.a*pos.x + back.b*pos.y + back.d) / back.c;
         depthX = (rght.b*pos.y + rght.c*pos.z + rght.d) / rght.a;
         depthOffsetZ = fabsf(fabsf(depthZ) - pos.z);
         depthOffsetX = fabsf(depthX) - pos.x;
         depthOffset = sqrt(depthOffsetX*depthOffsetX + depthOffsetZ*depthOffsetZ);
         norms = glm::normalize(glm::vec3(depthOffsetX,0.0f,-depthOffsetZ));
         mountPos.z = fabsf(depthZ) + depthOffsetZ/depthOffset;
         mountPos.x = fabsf(depthX) - depthOffsetX/depthOffset;
         break;
      case MOUNT_BACK:
         mountPos.z = (back.a*pos.x + back.b*pos.y + back.d) / -back.c + 1.0f;
         norms = glm::vec3(0,0,-1.0f);
         break;
      case MOUNT_BL:
         depthZ = (back.a*pos.x + back.b*pos.y + back.d) / back.c;
         depthX = (lft.b*pos.y + lft.c*pos.z + lft.d) / lft.a;
         depthOffsetX = fabsf(depthX) - pos.x;
         depthOffsetZ = fabsf(depthZ) - pos.z;
         depthOffset = sqrt(depthOffsetX*depthOffsetX + depthOffsetZ*depthOffsetZ);
         norms = glm::normalize(glm::vec3(depthOffsetX,0.0f,depthOffsetZ));
         mountPos.z = fabsf(depthZ) + depthOffsetZ/depthOffset;
         mountPos.x = fabsf(depthX) + depthOffsetX/depthOffset;
         break;
      case MOUNT_LEFT:
         mountPos.x = (lft.b*pos.y + lft.c*pos.z + lft.d) / -lft.a + 1.0f;
         norms = glm::vec3(-1.0f,0,0);
         break;
      case MOUNT_LF:
         depthZ = (front.a*pos.x + front.b*pos.y + front.d) / front.c;
         depthX = (lft.b*pos.y + lft.c*pos.z + lft.d) / lft.a;
         depthOffsetX = fabsf(fabsf(depthX) - pos.x);
         depthOffsetZ = fabsf(depthZ) - pos.z;
         depthOffset = sqrt(depthOffsetX*depthOffsetX + depthOffsetZ*depthOffsetZ);
         norms = glm::normalize(glm::vec3(-depthOffsetX,0.0f,depthOffsetZ));
         mountPos.z = fabsf(depthZ) - depthOffsetZ/depthOffset;
         mountPos.x = fabsf(depthX) + depthOffsetX/depthOffset;
         //printf("depth:(%f,0.0,%f), offset(%f,0.0,%f) / %f\n",depthX,depthZ,depthOffsetX,depthOffsetZ,depthOffset);
         break;
      default:
         break;
      }
      //printf("depth: %f on side %d with normal: (%f, %f, %f) at coords (%f, %f) with position (%f %f %f)\n",depthOffset, side,norms.x,norms.y,norms.z, x, y, roughPos.x, roughPos.y, roughPos.z);
   //}
   printf("locked on to: (%f,%f,%f)\n",mountPos.x,mountPos.y,mountPos.z);
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
   float x,y,z, off, f, r, b, l;
   off = (float)MOUNT_WIDTH;
   /*x = ((float)MOUNT_WIDTH - pos.x) * ((float)IMG_MAX_X / MOUNT_WIDTH);
   y = (((float)MOUNT_HEIGHT - pos.y) * (((float)IMG_MAX_Y - (float)IMG_MIN_Y) / (float)MOUNT_HEIGHT)) + (float)IMG_MIN_Y;
   y = interpolateDepth(x,y,0,norms);
   x = 30.0f;
   z = 30.0f;
   off -= (1.0 - y / 255.0) * off;*/
   f = front.a*pos.x + front.b*pos.y + front.c*pos.z + front.d;
   r = rght.a*pos.x + rght.b*pos.y + rght.c*pos.z + rght.d;
   b = back.a*pos.x + back.b*pos.y + back.c*pos.z + back.d;
   l = lft.a*pos.x + lft.b*pos.y + lft.c*pos.z + lft.d;
   //Determine which mountain side we're on
   if (f >= 0.0 && r >= 0.0) side = MOUNT_FR;
   else if (f >= 0.0 && l >= 0.0) side = MOUNT_LF;
   else if (b >= 0.0 && l >= 0.0) side = MOUNT_BL;
   else if (b >= 0.0 && r >= 0.0) side = MOUNT_RB;
   else if (f >= 0.0) side = MOUNT_FRONT;
   else if (r >= 0.0) side = MOUNT_RIGHT;
   else if (b >= 0.0) side = MOUNT_BACK;
   else if (l >= 0.0) side = MOUNT_LEFT;
   printf("f:%f,r:%f,b:%f,l:%f\n",f,r,b,l);
   /*if (pos.x >= pos.z && pos.z < pos.y) {
      if (pos.x < pos.y*off/MOUNT_HEIGHT) {
         side = MOUNT_FR;
      } else if (pos.x >= off - pos.y) {
         side = MOUNT_LF;
      } else {
         side = MOUNT_FRONT;
      }
   } else if (pos.x >= pos.z && pos.z < off - pos.y) {
      side = MOUNT_LEFT;
   } else if (pos.x >= pos.z) {
      side = MOUNT_BL;
   } else if (pos.z > pos.x && pos.x < pos.y) {
      if (pos.z < pos.y) {
         side = MOUNT_FR;
      } else if (pos.z >= off - pos.y) {
         side = MOUNT_RB;
      } else {
         side = MOUNT_RIGHT;
      }
   } else if (pos.z > pos.x && pos.x < off - pos.y) {
      side = MOUNT_BACK;
   } else if (pos.z > pos.x) {
      side = MOUNT_BL;
   }*/
   return side;
}
#endif
