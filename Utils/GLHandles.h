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

class GLHandles
{
   public:
      int ShadeProg;
      GLint aPosition;
      GLint aNormal;
      GLuint aUV;
      GLuint uTexUnit;
      GLint uModelMatrix;
      GLint uViewMatrix;
      GLint uProjMatrix;
      GLint uNormMatrix;
      GLint uLightPos;
      GLint uLightColor;
      GLint uEyePos;
      GLint uMatAmb;
      GLint uMatDif;
      GLint uMatSpec;
      GLint uMatShine;
};

#endif /* defined(___76_Lab1__GLHandles__) */
