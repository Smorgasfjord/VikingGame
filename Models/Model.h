//
//  Model.h
//  levelBuilder
//
//  Created by Taylor Woods on 4/15/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef __levelBuilder__Model__
#define __levelBuilder__Model__

#include <iostream>
#ifdef __APPLE__
#include <OPENGL/gl.h>
#endif

class Model
{
   public:
      GLuint NormalBuffObj;
      GLuint BuffObj;
      GLuint IndxBuffObj;
      GLuint TexBuffObj;
      int iboLen;
      static Model init_Ground(float g_groundY);
      static Model init_Mountain();
      static Model init_Platform();
      static Model init_Bjorn();
      static Model init_Hammer();
};

#endif /* defined(__levelBuilder__Model__) */
