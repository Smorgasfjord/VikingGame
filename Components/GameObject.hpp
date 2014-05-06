//
//  GameObject.h
//  476_Lab1
//
//  Created by Taylor Woods on 4/11/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef GAME_OBJ
#define GAME_OBJ

#include <GL\glew.h>
#include <Time.h>

#include <Windows.h>
#include "../glm/glm.hpp"
#include "../Utils/GLHandles.h"
using namespace std;

class GameObject
{
   protected:
      glm::vec3 position;
      glm::vec3 size;
      float rotation;
      glm::vec3 velocity;
      virtual void step() = 0;
      virtual void draw() = 0;
      GLHandles handles;
      double lastUpdated;
};

#endif
