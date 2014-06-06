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
      //The standards
      GLint aPosition;
      GLint aNormal;
      GLint uEyePos;
      //Matrix Transforms
      GLint uModelMatrix;
      GLint uViewMatrix;
      GLint uProjMatrix;
      GLint uNormMatrix;
      //Texture
      GLuint aUV;
      GLuint uTexUnit;
      //Light
      GLint uLightPos;
      GLint uLightColor;
      //Material
      GLint uMatAmb;
      GLint uMatDif;
      GLint uMatSpec;
      GLint uMatShine;
      //Fog
      GLuint uFogUnit;
      GLuint uFogStrength;
      GLuint uWindVec;
      //Shadows
      GLuint shadowMapID;
      GLuint depthMatrixID;
      GLuint depthBiasID;
};

#endif /* defined(___76_Lab1__GLHandles__) */
