#define GLM_SWIZZLE
#include "chunks.h"

bool operator==(const ChunkData a, const ChunkData b) {
   return a.x == b.x && a.y == b.y && a.z == b.z;
}
bool operator!=(const ChunkData a, const ChunkData b) {
   return !(a==b);
}
bool operator<(const ChunkData a, const ChunkData b) {
   if (a.x != b.x)
      return a.x < b.x;
   if (a.y != b.y)
      return a.y < b.y;
   return a.z < b.z;
}
bool operator==(const ObjData a, const ObjData b) {
   return a.obj == b.obj && a.nod == b.nod && a.mesh == b.mesh && a.tri == b.tri;
}
bool operator!=(const ObjData a, const ObjData b) {
   return !(a==b);
}
bool operator<(const ObjData a, const ObjData b) {
   if (a.obj != b.obj)
      return a.obj < b.obj;
   if (a.nod != b.nod)
      return a.nod < b.nod;
   if (a.mesh != b.mesh)
      return a.mesh < b.mesh;
   return a.tri < b.tri;
}

bool containedIn(glm::vec3 pt, glm::vec3 min, glm::vec3 max) {
   return pt.x >= min.x && pt.y >= min.y && pt.z >= min.z
          && pt.x <= max.x && pt.y <= max.y && pt.z <= max.z;
}

glm::vec3 nextChunk(glm::vec3 pos, glm::vec3 ray, float scale) {
   float x,y,z, xdis,ydis,zdis, temp;
   glm::vec3 rayNorm = ray / glm::length(ray);

   if (ray.x < 0.0) {
      temp = -APP * APP;
      x = floor(pos.x*scale) - pos.x * scale;
   }
   else {
      temp = APP * APP;
      x = ceil(pos.x*scale) - pos.x * scale;
   }
   xdis = (x != 0.0 ? x : temp) / (rayNorm.x != 0.0 ? rayNorm.x : temp);
   if (ray.y < 0.0) {
      temp = -APP * APP;
      y = floor(pos.y*scale) - pos.y * scale;
   }
   else {
      temp = APP * APP;
      y = ceil(pos.y*scale) - pos.y * scale;
   }
   ydis = (y != 0.0 ? y : temp) / (rayNorm.y != 0.0 ? rayNorm.y : temp);
   if (ray.z < 0.0) {
      temp = -APP * APP;
      z = floor(pos.z*scale) - pos.z * scale;
   }
   else {
      temp = APP * APP;
      z = ceil(pos.z*scale) - pos.z * scale;
   }
   zdis = (z != 0.0 ? z : temp) / (rayNorm.z != 0.0 ? rayNorm.z : temp);
   if (xdis < APP) xdis = APP;
   if (ydis < APP) ydis = APP;
   if (zdis < APP) zdis = APP;
   if (xdis < ydis) {
      if (xdis < zdis) {
         return pos + rayNorm * xdis * BOOST / scale;
      }
      else {
         return pos + rayNorm * zdis * BOOST / scale;
      }
   }
   else {
      if (ydis < zdis) {
         return pos + rayNorm * ydis * BOOST / scale;
      }
      else { //y = z (meet at edge or corner) or y > z
         return pos + rayNorm * zdis * BOOST / scale;
      }
   }
}

void ChunkWorld::print() {
   ChunkData idx;

   for (int i = -scale.z; i < scale.z; i++) {
      idx.z = i;
      for (int j = scale.y - 1; j >= -scale.y; j--) {
         idx.y = j;
         for (int k = -scale.x; k < scale.x; k++) {
            idx.x = k;
            if (chunkMap.count(idx)) std::cout << "@ ";
            else std::cout << ". ";
         }
         std::cout << "\n";
      }
      std::cout << "\n";
   }
}

Chunk & ChunkWorld::findChunk(float x, float y, float z) {
   ChunkData idx;
   idx.x = (int)floor(x);
   idx.y = (int)floor(y);
   idx.z = (int)floor(z);
   if (chunkMap.count(idx))
      return chunkMap[idx];
   return invalidC;
}

Chunk & ChunkWorld::addChunk(float x, float y, float z) {
   ChunkData idx;
   Chunk temp(floor(x), floor(y), floor(z));
   idx.x = (int)floor(x);
   idx.y = (int)floor(y);
   idx.z = (int)floor(z);
   chunkMap.insert(std::pair<ChunkData,Chunk>(idx,temp));
   return findChunk(x,y,z);
}

MicroChunk * ChunkWorld::findMicroChunk(float x, float y, float z) {
   ChunkData idx;
   idx.x = (int)floor(x*CHUNK_SIZE);
   idx.y = (int)floor(y*CHUNK_SIZE);
   idx.z = (int)floor(z*CHUNK_SIZE);
   if (uChunkMap.count(idx))
      return &uChunkMap[idx];
   return &invalidM;
}

MicroChunk * ChunkWorld::addMicroChunk(float x, float y, float z) {
   ChunkData idx;
   MicroChunk temp(floor(x*CHUNK_SIZE), floor(y*CHUNK_SIZE), floor(z*CHUNK_SIZE));
   idx.x = (int)floor(x*CHUNK_SIZE);
   idx.y = (int)floor(y*CHUNK_SIZE);
   idx.z = (int)floor(z*CHUNK_SIZE);
   uChunkMap.insert(std::pair<ChunkData,MicroChunk>(idx,temp));
   if (!findChunk(x,y,z).isValid()) {
      addChunk(x,y,z);
   }
   return findMicroChunk(x,y,z);
}

glm::mat4 ChunkWorld::accumTransform(ObjectNode *node, glm::mat4 cumulative, int & currNod, int targetNode) {
   glm::mat4 current = node->state.transform * cumulative, ret;
   currNod++;
   if (currNod == targetNode) {
      return current;
   }
   for (int j = 0; j < node->children.size(); j++) { 
      ret = accumTransform(&(node->children[j]), current, currNod, targetNode);
      if (!glm::isIdentity(ret,1.0f)) {
         return ret;
      }
   }
   return glm::mat4();
}

glm::mat4 ChunkWorld::findTransform(ObjData dat) {
   GameObject *mesh = &(objects[dat.obj]);
   int currNod = -1;
   return accumTransform(&(mesh->model), glm::mat4(), currNod, dat.nod);
}

glm::vec3 ChunkWorld::findCollisionPoint(glm::vec3 ray, glm::vec3 eye, ObjData dat, glm::mat4 trans) {
   float beta, gamma, traceLength, meta, eihf, gfdi, dheg, akjb, jcal, blkc;
   glm::vec3 vert1, vert2, vert3, mvert1, mvert2, mvert3, face, abc, def, ghi, jkl;


   /*const float *pSource = (const float*)glm::value_ptr(trans);
   for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j)
         printf("%f ", pSource[i+j*4]);
      printf("\n");
   }*/
   //printf("ray: (%f,%f,%f)  eye: (%f,%f,%f)\n",ray.x,ray.y,ray.z,eye.x,eye.y,eye.z);
   face = models[dat.obj][dat.mesh].faces[dat.tri];
   vert1 = models[dat.obj][dat.mesh].verts[(int)face.x];
   vert2 = models[dat.obj][dat.mesh].verts[(int)face.y];
   vert3 = models[dat.obj][dat.mesh].verts[(int)face.z];
   //printf("original positions: (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)\n", 
   //      vert1.x,vert1.y,vert1.z,vert2.x,vert2.y,vert2.z,vert3.x,vert3.y,vert3.z);

   mvert1 = (trans * glm::vec4(vert1,1.0)).xyz();
   mvert2 = (trans * glm::vec4(vert2,1.0)).xyz();
   mvert3 = (trans * glm::vec4(vert3,1.0)).xyz();
   //printf("moved positions: (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)\n", 
   //      mvert1.x,mvert1.y,mvert1.z,mvert2.x,mvert2.y,mvert2.z,mvert3.x,mvert3.y,mvert3.z);

   abc = mvert1 - mvert2;
   def = mvert1 - mvert3;
   ghi = ray;
   jkl = mvert1 - eye;

   eihf = def.y*ghi.z-ghi.y*def.z;
   gfdi = ghi.x*def.z-def.x*ghi.z;
   dheg = def.x*ghi.y-def.y*ghi.x;
   akjb = abc.x*jkl.y-jkl.x*abc.y;
   jcal = jkl.x*abc.z-abc.x*jkl.z;
   blkc = abc.y*jkl.z-jkl.y*abc.z;

   meta = abc.x*eihf + abc.y*gfdi + abc.z*dheg;
   //cout << "checking collision";
   traceLength = (def.z*akjb + def.y*jcal+def.x*blkc) / meta;
   //cout << "possible collision length: " << traceLength << "\n";
   if (traceLength > 0.0 || -traceLength > glm::length(ray))
      return glm::vec3(2.0*COLL_LIMIT);
   //cout << ".";
   gamma = (ghi.z*akjb + ghi.y*jcal+ghi.x*blkc) / meta;
   //cout << "gamma: " << gamma << "\n";
   if (gamma <= 0.0 || gamma >= 1.0)
      return glm::vec3(2.0*COLL_LIMIT);
   //cout << ".";
   beta = (jkl.x*eihf + jkl.y*gfdi+jkl.z*dheg) / meta;
   //cout << "beta: " << beta << "\n";
   if (beta <= 0.0 || beta >= 1.0 - gamma)
      return glm::vec3(2.0*COLL_LIMIT);
   //cout << ".";

   return glm::vec3(fabsf(traceLength), beta, gamma);
}

glm::vec3 ChunkWorld::interpolateNormal(float beta, float gamma, ObjData dat, glm::mat4 trans) {
   glm::vec3 face, norm1, norm2, norm3, mnorm1, mnorm2, mnorm3;

   trans = glm::transpose(glm::inverse(trans));
   face = models[dat.obj][dat.mesh].faces[dat.tri];
   mnorm1 = models[dat.obj][dat.mesh].norms[(int)face.x];
   mnorm2 = models[dat.obj][dat.mesh].norms[(int)face.y];
   mnorm3 = models[dat.obj][dat.mesh].norms[(int)face.z];
   
   if (mnorm1.x == mnorm2.x && mnorm2.x == mnorm3.x) 
      return glm::normalize(mnorm1 * glm::vec3(1.0,0.0f,0.0));
   if (mnorm1.y == mnorm2.y && mnorm2.y == mnorm3.y) 
      return glm::normalize(mnorm1 * glm::vec3(0.0,1.0f,0.0));
   if (mnorm1.z == mnorm2.z && mnorm2.z == mnorm3.z) 
      return glm::normalize(mnorm1 * glm::vec3(0.0,0.0f,1.0));
   return glm::normalize(mnorm1+mnorm2+mnorm3);//mnorm1 * (1.0f-beta-gamma) + mnorm2 * beta + mnorm3 * gamma;
}

CollisionData ChunkWorld::checkMeshCollision(const BufferContents & geom, glm::mat4 newTrans, glm::mat4 oldTrans, ObjData & dat) {
   MicroChunk *step;
   CollisionData ret;
   ObjData cDat;
   glm::vec3 cPoint, cAngle, cNormal;
   glm::vec3 newTransVert;
   glm::vec3 oldTransVert;
   glm::vec3 move, actual, iter;
   for (int i = 0; i < geom.verts.size(); i++) {
      dat.tri = i;
      newTransVert = (newTrans * glm::vec4(geom.verts[i],1.0f)).xyz();
      oldTransVert = (oldTrans * glm::vec4(geom.verts[i],1.0f)).xyz();
      move = newTransVert - oldTransVert;
      iter = oldTransVert;
      //while iterator has not reached the end of the line
      while (glm::dot(move,newTransVert - iter) >= 0.0) {
         step = findMicroChunk(iter.x, iter.y, iter.z);
         if (step->isValid()) {
            for (map<ObjData,glm::vec3>::iterator it=step->objects.begin(); it!=step->objects.end(); ++it) {
               if (objects[it->first.obj].collisionGroup != objects[dat.obj].collisionGroup) {
                  cDat = it->first;
                  move = newTransVert - oldTransVert;
                  cPoint = findCollisionPoint(glm::normalize(move),oldTransVert,cDat,findTransform(cDat));
                  if (cPoint.x != cPoint.x || cPoint.x > COLL_LIMIT) {
//                  printf("min: %f,%f,%f\n", temp->minBound.x,temp->minBound.y,temp->minBound.z);
//                  printf("max: %f,%f,%f\n", temp->maxBound.x,temp->maxBound.y,temp->maxBound.z);
                     continue;
                  }
                  cNormal = interpolateNormal(cPoint.y, cPoint.z, cDat, oldTrans);
                  actual = glm::normalize(move) * cPoint.x;
                  ret = CollisionData(cDat, dat, glm::vec3(oldTransVert) + actual, actual, cNormal, move);
                  return ret;
               }
            }
         }
         //increment by normalized line in CHUNK_SIZE units
         iter = nextChunk(iter, move, CHUNK_SIZE);
      }
   }
   return ret;
}

CollisionData ChunkWorld::checkNodeCollision(ObjectNode & newNod, ObjectNode & oldNod, const vector<BufferContents> & geom, glm::mat4 newCumulative, glm::mat4 oldCumulative, ObjData & dat) {
   glm::mat4 newCurrent = newNod.state.transform * newCumulative;
   glm::mat4 oldCurrent = oldNod.state.transform * oldCumulative;
   CollisionData ret;
   dat.nod++;
   for (int i = 0; i < newNod.meshes.size(); i++) {
      dat.mesh = i;
      ret = checkMeshCollision(geom[newNod.meshes[i].meshIdx], newCurrent, oldCurrent, dat);
      if (ret.hitObj.obj >= 0) {
         return ret;
      }
   }
   //SOMETHING IN HERE IS BREAKING, i think its if oldNod has no children
   for (int j = 0; j < newNod.children.size(); j++) { 
      ret = checkNodeCollision(newNod.children[j], oldNod.children[j], geom, newCurrent, oldCurrent, dat);
      if (ret.hitObj.obj >= 0) {
         return ret;
      }
   }
   return ret;
}

CollisionData ChunkWorld::checkForCollision(GameObject *obj, int objIndex) {
   GameObject & old = objects[objIndex];
   ObjData dat;
   dat.obj = objIndex;
   dat.nod = -1;
   return checkNodeCollision(obj->model, old.model, models[objIndex], glm::mat4(1.0f), glm::mat4(1.0f), dat);
}


void ChunkWorld::traceLine(glm::vec3 start, glm::vec3 end, ObjData dat) {
   glm::vec3 line = end - start;
   glm::vec3 iter = start;
   MicroChunk *step;
   //while iterator has not reached the end of the line
   while (glm::dot(line,end - iter) >= 0.0) {
      step = findMicroChunk(iter.x, iter.y, iter.z);
      if (!step->isValid()) {
         step = addMicroChunk(iter.x, iter.y, iter.z);
      }
      if (!step->objects.count(dat)) {
         step->objects.insert(std::pair<ObjData,glm::vec3>(dat, glm::vec3()));
         objectMap[dat.obj].push_back(step->index);
      }
      //increment by normalized line in CHUNK_SIZE units
      iter = nextChunk(iter, line, CHUNK_SIZE);
   }
}

void ChunkWorld::traceTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, ObjData check) {
   glm::vec3 line = v2 - v1;
   glm::vec3 iter = v1;

   while (glm::dot(line, v2 - iter) > 0.0) {
      traceLine(iter, v3, check);
      iter = nextChunk(iter, line, CHUNK_SIZE);
   }
}

void ChunkWorld::traceMesh(const BufferContents & geom, glm::mat4 trans, ObjData dat) {
   glm::vec4 vert1, vert2, vert3;
   dat.tri = 0;
   for (int i = 0; i < geom.faces.size(); i++) {
      vert1 = trans * glm::vec4(geom.verts[(int)geom.faces[i].x], 1.0);
      vert2 = trans * glm::vec4(geom.verts[(int)geom.faces[i].y], 1.0);
      vert3 = trans * glm::vec4(geom.verts[(int)geom.faces[i].z], 1.0);
      traceTriangle(glm::vec3(vert1), glm::vec3(vert2), glm::vec3(vert3), dat);
      dat.tri++;
   }
}

int ChunkWorld::traceNode(ObjectNode *node, const vector<BufferContents> & geom, glm::mat4 cumulative, ObjData dat) {
   glm::mat4 current = node->state.transform * cumulative;
   dat.nod++;
   for (int i = 0; i < node->meshes.size(); i++) {
      dat.mesh = i;
      traceMesh(geom[node->meshes[i].meshIdx], current, dat);
   }
   for (int j = 0; j < node->children.size(); j++) { 
      dat.nod = traceNode(&(node->children[j]), geom, current, dat);
   }
   return dat.nod;
}

void ChunkWorld::depopulate(int objIndex) {
   MicroChunk temp;
   bool done = false;
   glm::vec3 indx;
   ChunkData dat;
   map<ObjData,glm::vec3>::iterator it;
   
   for (int i = 0; i < objectMap[objIndex].size(); i++) {
      indx = objectMap[objIndex][i];
      dat.x = (int)indx.x;
      dat.y = (int)indx.y;
      dat.z = (int)indx.z;
      temp = uChunkMap[dat];
      while(!done)
      {
         done = true; //works now
         for (it=temp.objects.begin(); it != temp.objects.end(); ++it) {
            if (it->first.obj == objIndex) {
               temp.objects.erase(it);
               done = false;
               break;
            }
         }
      }
   }
   objectMap[objIndex].clear();
}

void ChunkWorld::repopulate(GameObject* obj, int objIndex) {
   ObjData dat;
   depopulate(objIndex);
   dat.obj = objIndex;
   dat.nod = -1;
   traceNode(&(obj->model), models[objIndex], glm::mat4(1.0f), dat);
   obj->copyTo(objects[objIndex]);
}

//returns the index of the object --KEEP THIS--
int ChunkWorld::populate(GameObject *mesh, const vector<BufferContents> & geom) {
   ObjData dat;

   dat.obj = objCount;
   dat.nod = -1;
   traceNode(&(mesh->model), geom, glm::mat4(1.0f), dat);

   objects.push_back(mesh->copy());
   models.push_back(geom);
   objCount++;
   return (int)objects.size() - 1;
}
/*
int ChunkWorld::addMaterial(mat_t mat) {
   mat.objId = materials.size() - 1;
   materials.push_back(mat);
   return mat.objId;
}

int ChunkWorld::addLight(LightSource light) {
   lights.push_back(light);
   return lights.size() - 1;
};*/
