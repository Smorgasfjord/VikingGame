//
//  Model.cpp
//  476_Lab1
//
//  Created by Taylor Woods on 4/13/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#include "Model.h"
#include "..\Utils\CMeshLoaderSimple.h"

 Model Model::init_Ground(float g_groundY)
{
   Model mod;
   float GrndPos[] = {
      0, g_groundY, 0,
      0, g_groundY, 1,
      1, g_groundY, 1,
      1, g_groundY, 0,
   };
   
   unsigned short idx[] =
   {
      2, 1, 0,
      3, 2, 0,
   };
   
   float grndNorm[] =
   {
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
   };
   
   static GLfloat GrndTex[] = {
      0, 0,
      0, 1,
      1, 0,
      1, 1
   };
   
   mod.iboLen = 6;
   glGenBuffers(1, &mod.BuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.BuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.IndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mod.IndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.NormalBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.NormalBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(grndNorm), grndNorm, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.TexBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.TexBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

   return mod;
}

Model Model::init_Platform()
{
   Model mod;
   float Pos[] = {
      -1, -0.25, 0, //0
      -1, -0.25, 1, //1
      1, -0.25, 1,  //2
      1, -0.25, 0,  //3
      -1, 0.25, 0,  //4
      -1, 0.25, 1,  //5
      1, 0.25, 1,   //6
      1, 0.25, 0,   //7
   };
   
   unsigned short idx[] =
   {
      2, 1, 0, //Bottom face
      3, 2, 0,
      4, 5, 6, //top face
      7, 4, 6,
      4, 0, 1, //Left
      5, 4, 1,
      6, 3, 2, //Right
      7, 3, 6,
      5, 2, 1, //Front
      6, 2, 5,
      4, 3, 0, //Back
      7, 3, 4
   };
   
   float Norm[] =
   {
      0, 1, 0,
      0, 1, 0,
      1, 0, 0,
      1, 0, 0,
      0, 0, 1,
      0, 0, 1, //here
      0, 1, 0,
      0, 1, 0,
      1, 0, 0,
      1, 0, 0,
      0, 0, 1,
      0, 0, 1,
   };
   
   static GLfloat Tex[] = {
      0, 0,
      0, 1,
      1, 0,
      1, 1
   };
   
   mod.iboLen = 36;
   glGenBuffers(1, &mod.BuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.BuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Pos), Pos, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.IndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mod.IndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.NormalBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.NormalBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Norm), Norm, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.TexBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.TexBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Tex), Tex, GL_STATIC_DRAW);
   
   return mod;
}

Model Model::init_Mountain()
{
   Model mod;
   float Pos[] = {
      -30, 0, -30, //0
      -30, 0, 30,  //1
      30, 0, 30,   //2
      30, 0, -30,  //3
      0, 30, 0     //4
   };
   
   unsigned short idx[] =
   {
      2, 1, 0, //Bottom face
      3, 2, 0,
      4, 1, 0, //Left
      4, 2, 1, //Front
      4, 3, 2, //Right
      4, 0, 3 //Back
   };
   
   float Norm[] =
   {
      0, -1, 0,
      0, -1, 0,
      -1, 1, 0,
      0, 1, 1,
      1, 1, 0,
      0, 1, -1
   };
   
   static GLfloat Tex[] = {
      0, 0,
      0, 1,
      1, 0,
      1, 1
   };
   
   mod.iboLen = 36;
   glGenBuffers(1, &mod.BuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.BuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Pos), Pos, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.IndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mod.IndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.NormalBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.NormalBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Norm), Norm, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.TexBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.TexBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Tex), Tex, GL_STATIC_DRAW);
   
   return mod;
   
}

Model Model::init_Bjorn()
{
   Model mod;
   GLuint colBuffObj;
   CMeshLoader::loadVertexBufferObjectFromMesh("gameguy1k.m", mod.iboLen,
                                               mod.BuffObj, colBuffObj, mod.NormalBuffObj);
   return mod;
}

Model Model::init_Hammer()
{
   Model mod;
   float Pos[] = {
      -.5, -0.1, -.1, //0
      -.5, -0.1, .1, //1
      .5, -0.1, .1,  //2
      .5, -0.1, -.1,  //3
      -.5, 0.1, -.1,  //4
      -.5, 0.1, .1,  //5
      .5, 0.1, .1,   //6
      .5, 0.1, -.1,   //7
   };
   
   unsigned short idx[] =
   {
      2, 1, 0, //Bottom face
      3, 2, 0,
      4, 5, 6, //top face
      7, 4, 6,
      4, 0, 1, //Left
      5, 4, 1,
      6, 3, 2, //Right
      7, 3, 6,
      5, 2, 1, //Front
      6, 2, 5,
      4, 3, 0, //Back
      7, 3, 4
   };
   
   float Norm[] =
   {
      0, 1, 0,
      0, 1, 0,
      1, 0, 0,
      1, 0, 0,
      0, 0, 1,
      0, 0, 1,
      0, 1, 0,
      0, 1, 0,
      1, 0, 0,
      1, 0, 0,
      0, 0, 1,
      0, 0, 1,
   };
   
   static GLfloat Tex[] = {
      0, 0,
      0, 1,
      1, 0,
      1, 1
   };
   
   mod.iboLen = 36;
   glGenBuffers(1, &mod.BuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.BuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Pos), Pos, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.IndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mod.IndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.NormalBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.NormalBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Norm), Norm, GL_STATIC_DRAW);
   
   glGenBuffers(1, &mod.TexBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mod.TexBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Tex), Tex, GL_STATIC_DRAW);
   
   return mod;
}