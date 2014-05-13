//
//  Bjorn.h
//  MyGame
//
//  Created by Taylor Woods on 5/3/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef __MyGame__Bjorn__
#define __MyGame__Bjorn__

#include <iostream>
#define DEBUG_GAME 0

#ifdef _WIN32
#include <time.h>
#include <GL\glew.h>
#endif

#define GLFW_INCLUDE_GLU
#ifdef __unix__
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#else
#include "glfw3.h"
#endif

#include "GameObject.h"
#include "../Utils/World.h"
#include "../Audio/Sound.h"


class Bjorn : public GameObject
{
   public:
      void step();
      Bjorn();
      ~Bjorn();
      Bjorn(glm::vec3 pos, GLHandles hand, GameModel *model, World world);
      void moveRight();
      void moveLeft();
      void jump();
      void launch(float angle);
      void suspend();
      void unsuspend();
   private:
      void SetModel(glm::vec3 loc, glm::vec3 size);
      bool jumping;
      bool suspended;
      World world;
      Model mod;
      float mass;
      float gravity;
      int mountainSide;
};

#endif /* defined(__MyGame__Bjorn__) */
