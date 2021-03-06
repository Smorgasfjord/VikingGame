//
//  World.cpp
//  MyGame
//
//  Created by Taylor Woods on 5/4/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#include "World.h"

World::~World()
{
}

World::World()
{
   
}

World::World(std::vector<Platform> plats, GameModel* simplePlatformMod, Mountain mnt, GLHandles* handles, int shadeProg)
{
   platforms = plats;
   mount = mnt;
   this->handles = handles;
   ShadeProg = shadeProg;
   space = ChunkWorld(50,50,50);
   for (int i = 0; i < plats.size(); i++) {
      placeObject(&(plats[i]), simplePlatformMod);
      cout << "Platform " << i << " placed\n";
   }
}


/* helper function to set up material for shading */
void World::SetMaterial(int i) {
   glUseProgram(ShadeProg);
   switch (i) {
      case 0:
		  safe_glUniform3f(handles->uMatAmb, 0.2, 0.2, 0.2);
		  safe_glUniform3f(handles->uMatDif, 0.4, 0.4, 0.4);
		  safe_glUniform3f(handles->uMatSpec, 0.2, 0.2, 0.2);
		  safe_glUniform1f(handles->uMatShine, .2);
         break;
      case GROUND_MAT:
		  safe_glUniform3f(handles->uMatAmb, 0.1, 0.3, 0.1);
		  safe_glUniform3f(handles->uMatDif, 0.1, 0.3, 0.1);
		  safe_glUniform3f(handles->uMatSpec, 0.3, 0.3, 0.4);
		  safe_glUniform1f(handles->uMatShine, 1.0);
         break;
   }
}

void World::draw(int mountainSide)
{  
   std::vector<GameObject>objectsInScene;
#ifdef _WIN32
	//------------------------------Depth Buffer -------------------------
	/*
	glGenFramebuffers(1, &(handles->framBuff));
	glBindFramebuffer(GL_FRAMEBUFFER, (handles->framBuff));
	
	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	glGenTextures(1, &(handles->depthBuff));
	glBindTexture(GL_TEXTURE_2D, handles->depthBuff);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, handles->depthBuff, 0);

	glDrawBuffer(GL_NONE); // No color buffer is drawn to.

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return;

	glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);

	// Compute the MVP matrix from the light's point of view
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
		//^^changes this based on the eye position. vvvv this too. and light position (?)
	glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 depthModelMatrix = glm::mat4(1.0); //<-- not sure how this affects anything
	glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

	// Send our transformation to the currently bound shader,
	// in the "MVP" uniform
	safe_glUniformMatrix4fv(handles->depthMatrixID, glm::value_ptr(depthMVP));
	
	*/
	/*

	glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);

	// Compute the MVP matrix from the light's point of view
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
	glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 depthModelMatrix = glm::mat4(1.0);
	glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

	// Send our transformation to the currently bound shader,
	// in the "MVP" uniform
	glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0])
	*/

#endif

	safe_glEnableVertexAttribArray(handles->aPosition);
	safe_glEnableVertexAttribArray(handles->aNormal);
   
   SetMaterial(2);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   mount.draw();
   
   objectsInScene = cull(mountainSide);
   SetMaterial(0);
   for (std::vector<GameObject>::iterator it = objectsInScene.begin(); it != objectsInScene.end(); ++ it) {
      it->draw();
   }
   
   //clean up
   safe_glDisableVertexAttribArray(handles->aPosition);
   safe_glDisableVertexAttribArray(handles->aNormal);
}

float testPlane(glm::vec4 row1, glm::vec4 row2, glm::vec3 point)
{
   float value;
   glm::vec4 vec4Point = glm::vec4(point, 1);
   for (int i = 0; i < 4; i++) {
      value += (row1[i] + row2[i]) * vec4Point[i];
   }
   return value;
}

//Returns a vector of GameObjects which need to be drawn
std::vector<GameObject> World::cull(int mountainSide)
{
   vector<GameObject> objects;
   //Get mvp matrix
   glm::mat4 model, view, projection, mvp;
   glGetUniformfv(ShadeProg, handles->uViewMatrix, glm::value_ptr(view));
   glGetUniformfv(ShadeProg, handles->uModelMatrix, glm::value_ptr(model));
   glGetUniformfv(ShadeProg, handles->uProjMatrix, glm::value_ptr(projection));
   
   mvp = projection * view * model;
   cout << "First thing in mvp: " << mvp[0][0] << "\n";
   for (std::vector<Platform>::iterator it = platforms.begin(); it != platforms.end(); ++ it) {
      //This will cull any platforms on the opposite side of the mountain
      if (((mountainSide + it->mountainSide) % 2 != 0) || (mountainSide == it->mountainSide)) {
         //Not on the back side, check against each plane of view frustum
         //Negative Z
         if(testPlane(mvp[2], -mvp[3], it->getPos()) <= 0)
         {
            //Positive Z
            if(testPlane(mvp[2], mvp[3], it->getPos()) >= 0)
            {
               //Negative Y
               if(testPlane(mvp[1], -mvp[3], it->getPos()) <= 0)
               {
                  //Positive Y
                  if(testPlane(mvp[1], mvp[3], it->getPos()) >= 0)
                  {
                     //Negative X
                     if(testPlane(mvp[0], -mvp[3], it->getPos()) <= 0)
                     {
                        //Positive X
                        if(testPlane(mvp[0], mvp[3], it->getPos()) >= 0)
                        {
                           objects.push_back(*it);
                        }
                     }
                     
                  }
               }
            }
         }
         
      }
   }
   return objects;
}

//Given a position return the Y coordinate of the top of the platform located there
//if there is one, else return original pos.y
float World::getY(glm::vec3 pos)
{
   //Check for collision with platform
   for (std::vector<Platform>::iterator it = platforms.begin(); it != platforms.end(); ++ it) {
      if(it->detectCollision(pos))
         return it->getPos().y + .25;
   }
   return pos.y;
}

//Returns the starting point of the level, could be updated to include checkpoints
glm::vec3 World::getStart()
{
   return platforms[0].getPos();
}

int World::detectCollision(glm::vec3 pos)
{
   //Check for collision with platform
   for (std::vector<Platform>::iterator it = platforms.begin(); it != platforms.end(); ++ it) {
         if(it->detectCollision(pos))
            return PLATFORM_COLLISION;
   }
   return 0;
}

CollisionData World::checkCollision(GameObject *obj, int objIndex) {
   return space.checkForCollision(obj, objIndex);
}

int World::placeObject(GameObject *obj, GameModel *mod) {
   return space.populate(obj, mod->contents);
}

void World::updateObject(GameObject *obj, int objIndex) {
   space.repopulate(obj, objIndex);
}

void World::removeObject(int objIndex) {
   space.depopulate(objIndex);
}

GameObject & World::getObjectByIndex(int idx) {
   return space.objects[idx];
}
