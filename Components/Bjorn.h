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
      void step(double timeStep);
      Bjorn();
      ~Bjorn();
      Bjorn(glm::vec3 pos, GLHandles hand, GameModel *model, World * world);
      void moveRight(float t);
      void moveLeft(float t);
      void jump();
      void launch(float angle);
      void suspend();
      void unsuspend();
      void update(double timeStep);
      int mountainSide;
      bool facingRight;
      bool screamed;
      bool jumping;
      bool suspended;
      void save();
      void reset();
   private:
      void SetModel(glm::vec3 loc, glm::vec3 size);
      World * world;
      float mass;
      float gravity;
      bool s_facingRight;
      int s_mountainSide;
      Transform_t s_state;
};

#endif /* defined(__MyGame__Bjorn__) */
