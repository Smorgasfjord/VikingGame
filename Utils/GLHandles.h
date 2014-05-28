//
//  GLHandles.h
//  476_Lab1
//
//  Created by Taylor Woods on 4/11/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef ___76_Lab1__GLHandles__
#define ___76_Lab1__GLHandles__

#include <iostream>

#ifdef __APPLE__
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glew.h>
#include <GL/gl.h>
#endif
#ifdef _WIN32
#include <GL\glew.h>
#endif

class GLHandles
{
   public:
      int ShadeProg;
      GLuint aPosition;
      GLuint aNormal;
      GLuint aUV;
      GLuint uTexUnit;
      GLuint uModelMatrix;
      GLuint uViewMatrix;
      GLuint uProjMatrix;
      GLuint uNormMatrix;
      GLuint uLightPos;
      GLuint uLightColor;
      GLuint uEyePos;
      GLuint uMatAmb;
      GLuint uMatDif;
      GLuint uMatSpec;
      GLuint uMatShine;
      GLuint frameBuff;
      //GLuint depthBuff;
      GLuint depthBiasID;
      GLuint depthMatrixID;
      GLuint shadowMapID;
};

#endif /* defined(___76_Lab1__GLHandles__) */
