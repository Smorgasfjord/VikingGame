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

World::World(std::vector<Platform> plats, Mountain mnt, Model gndMod, GLHandles handles, int shadeProg)
{
   platforms = plats;
   mount = mnt;
   grndMod = gndMod;
   this->handles = handles;
   ShadeProg = shadeProg;
}

/* Set up matrices for ground plane */
void World::setGround(glm::vec3 loc)
{
   glm::mat4 ctm = glm::translate(glm::mat4(1.0f), loc);
   safe_glUniformMatrix4fv(handles.uModelMatrix, glm::value_ptr(ctm));
   safe_glUniformMatrix4fv(handles.uNormMatrix, glm::value_ptr(glm::mat4(1.0f)));
}

/* helper function to set up material for shading */
void World::SetMaterial(int i) {
   glUseProgram(ShadeProg);
   switch (i) {
      case 0:
         safe_glUniform3f(handles.uMatAmb, 0.2, 0.2, 0.2);
         safe_glUniform3f(handles.uMatDif, 0.4, 0.4, 0.4);
         safe_glUniform3f(handles.uMatSpec, 0.2, 0.2, 0.2);
         safe_glUniform1f(handles.uMatShine, .2);
         break;
      case GROUND_MAT:
         safe_glUniform3f(handles.uMatAmb, 0.1, 0.3, 0.1);
         safe_glUniform3f(handles.uMatDif, 0.1, 0.3, 0.1);
         safe_glUniform3f(handles.uMatSpec, 0.3, 0.3, 0.4);
         safe_glUniform1f(handles.uMatShine, 1.0);
         break;
   }
}

void World::draw()
{  
   //-------------------------------Ground Plane --------------------------
   safe_glEnableVertexAttribArray(handles.aPosition);
   safe_glEnableVertexAttribArray(handles.aNormal);
   SetMaterial(GROUND_MAT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   for (std::vector<glm::vec3>::iterator it = groundTiles.begin(); it != groundTiles.end(); ++ it) {
      setGround(glm::vec3(it->x, it->y, it->z));
      
      glBindBuffer(GL_ARRAY_BUFFER, grndMod.BuffObj);
      safe_glVertexAttribPointer(handles.aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grndMod.IndxBuffObj);
      
      safe_glEnableVertexAttribArray(handles.aNormal);
      glBindBuffer(GL_ARRAY_BUFFER, grndMod.NormalBuffObj);
      safe_glVertexAttribPointer(handles.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
      
      glDrawElements(GL_TRIANGLES, grndMod.iboLen, GL_UNSIGNED_SHORT, 0);
   }
   
   SetMaterial(2);
   mount.draw();
   
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   SetMaterial(0);
   for (std::vector<Platform>::iterator it = platforms.begin(); it != platforms.end(); ++ it) {
      it->draw();
   }
   
   //clean up
	safe_glDisableVertexAttribArray(handles.aPosition);
	safe_glDisableVertexAttribArray(handles.aNormal);
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

int World::detectCollision(glm::vec3 pos)
{
   //Check for collision with platform
   for (std::vector<Platform>::iterator it = platforms.begin(); it != platforms.end(); ++ it) {
         if(it->detectCollision(pos))
            return PLATFORM_COLLISION;
   }
   return 0;
}
