
#include "chunks.h"

bool containedIn(glm::vec3 pt, glm::vec3 min, glm::vec3 max) {
   return pt.x >= min.x && pt.y >= min.y && pt.z >= min.z
          && pt.x <= max.x && pt.y <= max.y && pt.z <= max.z;
}

/*glm::vec3 checkCollision(glm::vec3 ray, glm::vec3 eye, GameObject *mesh, int tri) {
   float beta, gamma, trace, meta, eihf, gfdi, dheg, akjb, jcal, blkc;
   glm::vec3 vert1, vert2, vert3, abc, def, ghi, jkl;

   vert1 = mesh->Vertices[mesh->Triangles[tri].vIdx1].pos;
   vert2 = mesh->Vertices[mesh->Triangles[tri].vIdx2].pos;
   vert3 = mesh->Vertices[mesh->Triangles[tri].vIdx3].pos;

   abc = vert1 - vert2;
   def = vert1 - vert3;
   ghi = ray;
   jkl = vert1 - eye;

   eihf = def.y*ghi.z-ghi.y*def.z;
   gfdi = ghi.x*def.z-def.x*ghi.z;
   dheg = def.x*ghi.y-def.y*ghi.x;
   akjb = abc.x*jkl.y-jkl.x*abc.y;
   jcal = jkl.x*abc.z-abc.x*jkl.z;
   blkc = abc.y*jkl.z-jkl.y*abc.z;

   meta = abc.x*eihf + abc.y*gfdi + abc.z*dheg;
   //cout << "checking collision";
   trace = (def.z*akjb + def.y*jcal+def.x*blkc) / meta;
   if (trace > 0.0 || fabsf(trace) > ray.length())
      return glm::vec3(2.0*COLL_LIMIT);
   //cout << ".";
   gamma = (ghi.z*akjb + ghi.y*jcal+ghi.x*blkc) / meta;
   if (gamma <= 0.0 || gamma >= 1.0)
      return glm::vec3(2.0*COLL_LIMIT);
   //cout << ".";
   beta = (jkl.x*eihf + jkl.y*gfdi+jkl.z*dheg) / meta;
   if (beta <= 0.0 || beta >= 1.0 - gamma)
      return glm::vec3(2.0*COLL_LIMIT);
   //cout << ".";

   return glm::vec3(fabsf(trace), beta, gamma);
}
*/
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

void ChunkWorld::traceLine(glm::vec3 start, glm::vec3 end, ObjData dat) {
   glm::vec3 line = end - start;
   glm::vec3 iter = start;
   glm::vec3 last = end - start;
   MicroChunk *step;
   //while iterator has not reached the end of the line
   while (glm::dot(line,end - iter) >= 0.0) {
      step = findMicroChunk(iter.x, iter.y, iter.z);
      if (!step->isValid()) {
         step = addMicroChunk(iter.x, iter.y, iter.z);
      }
      if (!step->objects.count(dat)) {
         step->objects.insert(std::pair<ObjData,glm::vec3>(dat, glm::vec3()));
      }
      //increment by normalized line in CHUNK_SIZE units
      iter = nextChunk(iter, line, CHUNK_SIZE);
   }
}

void ChunkWorld::traceTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, ObjData check) {
   glm::vec3 line = v2 - v1;
   glm::vec3 iter = v1;
   glm::vec3 last = v2 - v1;

   while (glm::dot(line, v2 - iter) > 0.0) {
      traceLine(iter, v3, check);
      iter = nextChunk(iter, line, CHUNK_SIZE);
   }
}

void ChunkWorld::traceMesh(const BufferContents & geom, glm::mat4 trans, ObjData dat) {
   glm::vec4 vert1, vert2, vert3;
   for (int i = 0; i < geom.faces.size(); i++) {
      vert1 = trans * glm::vec4(geom.verts[(int)geom.faces[i].x], 1.0);
      vert2 = trans * glm::vec4(geom.verts[(int)geom.faces[i].y], 1.0);
      vert3 = trans * glm::vec4(geom.verts[(int)geom.faces[i].z], 1.0);
      traceTriangle(glm::vec3(vert1), glm::vec3(vert2), glm::vec3(vert3), dat);
   }
}

int ChunkWorld::traceNode(ObjectNode *node, const vector<BufferContents> & geom, glm::mat4 cumulative, ObjData dat) {
   glm::mat4 current = node->state.transform * cumulative;
   for (int i = 0; i < node->meshes.size(); i++) {
      dat.mesh = i;
      traceMesh(geom[node->meshes[i].meshIdx], current, dat);
   }
   dat.nod++;
   for (int j = 0; j < node->children.size(); j++) { 
      dat.nod = traceNode(&(node->children[j]), geom, current, dat);
   }
   return dat.nod;
}

//returns the index of the object --KEEP THIS--
int ChunkWorld::populate(GameObject *mesh, const vector<BufferContents> & geom) {
   ObjData dat;

   dat.obj = objCount;
   dat.nod = 0;
   traceNode(&(mesh->model), geom, glm::mat4(1.0f), dat);

   objects.push_back(*mesh);
   models.push_back(geom);
   objCount++;
   return objCount - 1;
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
}*/
