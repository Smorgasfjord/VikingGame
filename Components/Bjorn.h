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

#ifdef __WIN32__
#include <GL\glew.h>
#pragma comment (lib, "glfw3.lib")
#pragma comment (lib, "glew32.lib")
#endif

#define GLFW_INCLUDE_GLU
#include "glfw3.h"

#include "GameObject.hpp"
#include "../Utils/World.h"
#include "../Models/Model.h"


class Bjorn : public GameObject
{
   public:
      void step();
      void draw();
      Bjorn();
      Bjorn(glm::vec3 pos, GLHandles hand, Model model, World world);
      glm::vec3 getPos();
      void moveRight();
      void moveLeft();
      void jump();
      void launch(float angle);
   private:
      void SetModel(glm::vec3 loc, glm::vec3 size);
      World world;
      Model mod;
      float mass = 100;
      float gravity = (float)(-.8);
};

#endif /* defined(__MyGame__Bjorn__) */
