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

World::World(std::vector<Platform> plats, GameModel* simplePlatformMod, Mountain mnt, GLHandles* handles)
{
   platforms = plats;
   mount = mnt;
   this->handles = handles;
   space = ChunkWorld(50,50,50);
   for (int i = 0; i < plats.size(); i++) {
      placeObject(&(plats[i]), simplePlatformMod);
   }
}


/* helper function to set up material for shading */
void World::SetMaterial(int i) {
   glUseProgram(handles->ShadeProg);
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

//This should be smarter
std::vector<GameObject*> World::getDrawn(int mountainSide)
{
   std::vector<GameObject*>objectsInScene = cull(mountainSide);
   objectsToDraw = objectsInScene;
   objectsInScene.push_back(&mount);
  
   return objectsInScene;
}

void World::draw(int mountainSide)
{
	safe_glEnableVertexAttribArray(handles->aPosition);
	safe_glEnableVertexAttribArray(handles->aNormal);
   
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   mount.draw();

   for (int i = 0; i < objectsToDraw.size(); i++) {
      objectsToDraw.at(i)->draw();
   }
   
   //clean up
   safe_glDisableVertexAttribArray(handles->aPosition);
   safe_glDisableVertexAttribArray(handles->aNormal);
}

float testPlane(glm::vec4 row1, glm::vec4 row2, glm::vec4 point, float radius)
{
   float value = 0;
   float A = row1[0] + row2[0];
   float B = row1[1] + row2[1];
   float C = row1[2] + row2[2];
   float normalize = glm::length(glm::vec3(A, B, C));

   for (int i = 0; i < 4; i++) {
      value += ((row1[i] + row2[i]) * point[i]) / normalize;
   }

   return value + radius;
}

//Returns a vector of GameObjects which need to be drawn
std::vector<GameObject*> World::cull(int mountainSide)
{
   vector<GameObject*> objects;
   //Get mvp matrix
   glm::mat4 model, view, projection, mvp;
   float radius;
   glm::vec4 point;
   glm::vec4 platformCenter = glm::vec4(0, 0, 0, 1);
   //View and projection matrices are shared by all objects
   glGetUniformfv(handles->ShadeProg, handles->uViewMatrix, glm::value_ptr(view));
   glGetUniformfv(handles->ShadeProg, handles->uProjMatrix, glm::value_ptr(projection));
   
   for (int i = 0; i < platforms.size(); i++) {
      //This will cull any platforms on the opposite side of the mountain
      if (((mountainSide + platforms.at(i).mountainSide) % 4 != 0) || (mountainSide == platforms.at(i).mountainSide)) {
         //Not on the back side, check against each plane of view frustum
         //Get the model transform for this Object
         model = platforms.at(i).model.state.transform;
         radius = max(platforms.at(i).getScale().x, platforms.at(i).getScale().z);
         mvp = projection * view * model;
         //Negative Z
         if(testPlane(glm::row(mvp, 2), glm::row(mvp, 3), platformCenter, radius) > 0)
         {
            //Positive Z
            if(testPlane(-glm::row(mvp, 2), glm::row(mvp, 3), platformCenter, radius) > 0)
            {
               //Negative Y
               if(testPlane(glm::row(mvp, 1), glm::row(mvp, 3), platformCenter, radius) > 0)
               {
                  //Positive Y
                  if(testPlane(-glm::row(mvp, 1), glm::row(mvp, 3), platformCenter, radius) > 0)
                  {
                     //Negative X
                     if(testPlane(glm::row(mvp, 0), glm::row(mvp, 3), platformCenter, radius) > 0)
                     {
                        //Positive X
                        if(testPlane(-glm::row(mvp, 0), glm::row(mvp, 3), platformCenter, radius) > 0)
                        {
                           objects.push_back(&platforms.at(i));
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
