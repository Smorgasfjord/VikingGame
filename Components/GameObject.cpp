#ifndef GAME_OBJECT_CPP
#define GAME_OBJECT_CPP

#include "GameObject.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

using namespace glm;

void initGameObjState(Transform_t *state) {
   state->pos = state->orient = vec3(0.0);
   state->scale = vec3(1.0);
   state->translate = state->scaling = state->rotation = state->transform
      = mat4(1.0);
}

bool containedIn(vec3 pt, vec3 min, vec3 max) {
   return pt.x >= min.x && pt.y >= min.y && pt.z >= min.z
          && pt.x <= max.x && pt.y <= max.y && pt.z <= max.z;
}

vec3 GameObject::pos() {
   return model.state.pos;
}

vec3 GameObject::vel() {
   return model.state.velocity;
}

void GameObject::trans(float x, float y, float z) {
   int i;
   mat4 inmesh, *outmesh;
   vec4 newPos;

   model.state.pos.x += x;
   model.state.pos.y += y;
   model.state.pos.z += z;

   inmesh = model.state.translate;
   outmesh = &model.state.translate;
   
   *outmesh = inmesh * translate(mat4(1.0f), vec3(x,y,z));
   model.state.transform = (*outmesh) * model.state.rotation * model.state.scaling;
}

void GameObject::rot(float x, float y, float z) {
   int i;
   vec3 center;
   vec4 newPos;
   mat4 movTrans, rotTrans, retTrans, inmesh, *outmesh;

   center = model.state.pos;
   inmesh = model.state.rotation;
   outmesh = &model.state.rotation;

//   updateRotation(x,y);
   movTrans = translate(mat4(1.0f), -center);
   retTrans = translate(mat4(1.0f), center);
   rotTrans = rotate(mat4(1.0f), x, vec3(0.0f, 1.0f, 0.0f));
   rotTrans = rotate(mat4(1.0f), y, vec3(1.0f, 0.0f, 0.0f))*rotTrans;
   rotTrans = rotate(mat4(1.0f), z, vec3(0.0f, 0.0f, 1.0f))*rotTrans;
   *outmesh = retTrans * rotTrans * movTrans * inmesh;

   model.state.transform = model.state.translate * (*outmesh) * model.state.scaling;
}

void GameObject::rescale(float x, float y, float z) {
   int i;
   vec3 center, vScale, currScale;
   vec4 newPos;
   mat4 movTrans, sTrans, retTrans, inmesh, *outmesh;
   //SBoundingBox temp;
   center = model.state.pos;
   vScale = vec3(x,y,z);
   currScale = model.state.scale;

   inmesh = model.state.scaling;
   outmesh = &model.state.scaling;
   
   movTrans = translate(mat4(1.0f), -center);
   retTrans = translate(mat4(1.0f), center);
   *outmesh = retTrans * scale(mat4(1.0f), vScale) * movTrans * inmesh;
   model.state.transform = model.state.translate * model.state.rotation * (*outmesh);

   currScale.x *= vScale.x;
   currScale.y *= vScale.y;
   currScale.z *= vScale.z;
   //temp = SBoundingBox(bounds.left * x, bounds.bottom * y, bounds.back * z);
   //temp.update(bounds.right * x, bounds.top * y, bounds.front * z);
   //bounds = temp;
}

void GameObject::setPhysProps(float mass, int gravAffect) {
   this->mass = mass;
   this->gravityAffected = gravAffect;
}

vec3 GameObject::applyForce(vec3 force) {
   vec3 deltaV;
   model.state.velocity += force / mass;
   return deltaV;
}

vec3 GameObject::addVelocity(vec3 vel) {
   vec3 forceApplied;
   model.state.velocity += vel;
   return forceApplied;
}

vec3 GameObject::setVelocity(vec3 vel) {
   vec3 forceApplied;
   model.state.velocity = vel;
   return forceApplied;
}

vec3 GameObject::applyTransform(mat4 tran) {
   vec3 forceApplied;
   model.state.transform *= tran;
   return forceApplied;
}

float GameObject::scaleMass(float scale) {
   return 1.0;
}

void ObjectNode::initialize(ModelNode *modNod) {
   ObjectMesh mesh;
   ObjectNode nod;
   for (int i = 0; i < modNod->meshes.size(); i++) {
      mesh = ObjectMesh(i, modNod->meshes[i].buffDat);
      meshes.push_back(mesh);
   }
   for (int j = 0; j < modNod->children.size(); j++) {
      nod = ObjectNode(modNod->name);
      nod.initialize(&modNod->children[j]);
      children.push_back(nod);
   }
   state.transform = modNod->transform;
}

void GameObject::initialize(GameModel *model, int modIdx, int collGroup, GLHandles handles) {
   ObjectNode nod;
   this->handles = handles;
   this->collisionGroup = collGroup;
   this->modelIdx = modIdx;
   nod = ObjectNode(model->rootNode.name);
   nod.initialize(&model->rootNode);
   this->model = nod;
}

void GameObject::draw() {
   //glUseProgram(handles.ShadeProg);
   model.render(handles, mat4(1.0f));
}

void ObjectNode::render(GLHandles handle, mat4 cumulative) {
   mat4 current = state.transform * cumulative;
   safe_glUniformMatrix4fv(handle.uModelMatrix, value_ptr(current));
   safe_glUniformMatrix4fv(handle.uNormMatrix, value_ptr(transpose(inverse(current))));

   for (int i = 0; i < meshes.size(); i++) {
      meshes[i].render(handle);
   }

   for (int j = 0; j < children.size(); j++) {
      children[j].render(handle, current);
   }
}

void ObjectMesh::render(GLHandles handle) {
  //Enable handles
   safe_glEnableVertexAttribArray(handle.aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, buffDat.nbo);
   safe_glVertexAttribPointer(handle.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
 
   safe_glEnableVertexAttribArray(handle.aUV);
   glBindBuffer(GL_ARRAY_BUFFER, buffDat.tbo);
   safe_glVertexAttribPointer(handle.aUV, 2, GL_FLOAT, GL_FALSE, 0, 0);
   
   safe_glEnableVertexAttribArray(handle.aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, buffDat.vbo);
   safe_glVertexAttribPointer(handle.aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glUniform1i(handle.uTexUnit,0);
   safe_glUniform3f(handle.uMatAmb, buffDat.mat.ambient[0], buffDat.mat.ambient[1], buffDat.mat.ambient[2]);
   safe_glUniform3f(handle.uMatDif, buffDat.mat.diffuse[0], buffDat.mat.diffuse[1], buffDat.mat.diffuse[2]);
   safe_glUniform3f(handle.uMatSpec, buffDat.mat.specular[0], buffDat.mat.specular[1], buffDat.mat.specular[2]);
   safe_glUniform1f(handle.uMatShine, buffDat.mat.shininess);
   //SetMaterial(0, handle.ShadeProg, handle);
   //glBindBufferRange(GL_UNIFORM_BUFFER, (GLuint)2, buffDat.uniformBlockIndex, 0, sizeof(struct MyMaterial));
   // bind texture
   glBindTexture(GL_TEXTURE_2D, buffDat.texIndex);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffDat.ibo);
   // draw!
   glDrawElements(GL_TRIANGLES, buffDat.numFaces, GL_UNSIGNED_INT, 0);
   //glDrawArrays(GL_TRIANGLES, 0, buffDat.numFaces);
   //clean up
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   //clean up
   safe_glDisableVertexAttribArray(handle.aPosition);
   safe_glDisableVertexAttribArray(handle.aNormal);
   safe_glDisableVertexAttribArray(handle.aUV);
}

void GameObject::update(double timeStep) {
   if (!grounded && gravityAffected && timeStep > 0) {
      model.state.velocity += vec3(0.0, -20.0, 0.0);
   }
   vec3 mov = model.state.velocity * (float)timeStep;
   trans(mov.x,mov.y,mov.z);
}
#endif
