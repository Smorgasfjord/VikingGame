#ifndef GAME_OBJECT_CPP
#define GAME_OBJECT_CPP

#include "GameObject.h"

using namespace glm;

#ifdef _WIN32
#include <GL\glew.h>
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#endif


void initGameObjState(Transform_t *state) {
   state->pos = state->orient = vec3(0.0);
   state->scale = vec3(1.0);
   state->translate = state->scaling = state->rotation = state->transform
      = mat4(1.0);
}

void ObjectNode::initialize(ModelNode & modNod) {
   ObjectMesh mesh;
   ObjectNode nod;
   for (int i = 0; i < modNod.meshes.size(); i++) {
      mesh = ObjectMesh(modNod.meshes[i].mIdx, modNod.meshes[i].buffDat);
      meshes.push_back(mesh);
   }
   for (int j = 0; j < modNod.children.size(); j++) {
      nod = ObjectNode(modNod.children[j].name.c_str());
      nod.initialize(modNod.children[j]);
      children.push_back(nod);
   }
   state.transform = modNod.transform;
}

void GameObject::initialize(GameModel & model, int modIdx, int collGroup, GLHandles handles) {
   ObjectNode nod;
   this->handles = handles;
   this->collisionGroup = collGroup;
   this->modelIdx = modIdx;
   nod = ObjectNode(model.rootNode.name.c_str());
   nod.initialize(model.rootNode);
   this->model = nod;
}

void ObjectNode::copyTo(ObjectNode & node) {
   node.state = state;
   /*for (int i = 0; i < meshes.size(); i++) {
      meshes[i].copyTo(node.children[i]);
   }*/
   for (int j = 0; j < children.size(); j++) {
      children[j].copyTo(node.children[j]);
   }
}

void GameObject::copyTo(GameObject & obj) {
   obj.mass = mass;
   obj.gravityAffected = gravityAffected;
   obj.grounded = grounded;
   obj.collisionGroup = collisionGroup;
   obj.modelIdx = modelIdx;
   model.copyTo(obj.model);
}

ObjectNode & ObjectNode::copy() {
   ObjectNode *node = new ObjectNode(name.c_str());
   memcpy(&(node->state), &state, sizeof(state));
   for (int i = 0; i < meshes.size(); i++) {
      node->meshes.push_back(ObjectMesh(i, meshes[i].buffDat));
   }
   for (int j = 0; j < children.size(); j++) {
      node->children.push_back(children[j].copy());
   }

   return *node;
}

GameObject & GameObject::copy() {
   GameObject *obj = new GameObject(name.c_str());
   obj->collisionGroup = collisionGroup;
   obj->modelIdx = modelIdx;
   obj->model = model.copy();
   return *obj;
}

//------------------------------Get-ers-------------------------------

vec3 GameObject::getPos() {
   return model.state.pos;
}

vec3 GameObject::getVel() {
   return model.state.velocity;
}

vec3 GameObject::getRot() {
   return model.state.orient;
}

mat4 GameObject::getRotMat() {
   return model.state.rotation;
}

vec3 GameObject::getScale() {
   return model.state.scale;
}

Transform_t GameObject::getState() {
   return model.state;
}

//------------------------------Set-ers------------------------------

void GameObject::setPos(glm::vec3 pos)
{
   model.state.pos = pos;
   model.state.translate = translate(mat4(1.0f), pos);
   updateTransformMatrix();
}

void GameObject::setScale(glm::vec3 scale)
{
   model.state.scale = scale;
   model.state.scaling = glm::scale(mat4(1.0f), model.state.scale);
   updateTransformMatrix();
}

void GameObject::setRotation(glm::vec3 rot)
{
   model.state.orient = rot;
   model.state.rotation = rotate(mat4(1.0f), rot.x, vec3(1.0f, 0.0f, 0.0f));
   model.state.rotation *= rotate(mat4(1.0f), rot.y, vec3(0.0f, 1.0f, 0.0f));
   model.state.rotation *= rotate(mat4(1.0f), rot.z, vec3(0.0f, 0.0f, 1.0f));
   updateTransformMatrix();
}

void GameObject::setState(Transform_t state)
{
   model.state = state;
}

//-----------------------------Updaters------------------------------

//Move the object by the amount specified
void GameObject::moveBy(glm::vec3 amount) {
   model.state.pos += amount;
   
   model.state.translate *= translate(mat4(1.0f), amount);
   updateTransformMatrix();
}

//Rotate object by amount specified
//amount.x about the x axis, etc
void GameObject::rotateBy(glm::vec3 amount) {
   model.state.orient += amount;
   
   model.state.rotation *= rotate(mat4(1.0f), amount.x, vec3(1.0f, 0.0f, 0.0f));
   model.state.rotation *= rotate(mat4(1.0f), amount.y, vec3(0.0f, 1.0f, 0.0f));
   model.state.rotation *= rotate(mat4(1.0f), amount.z, vec3(0.0f, 0.0f, 1.0f));
   
   updateTransformMatrix();
}

void GameObject::scaleBy(glm::vec3 amount) {
   model.state.scale *= amount;
   
   model.state.scaling = glm::scale(mat4(1.0f), model.state.scale);
   updateTransformMatrix();
}

//------------------------------------Physics Stuff---------------------------
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

float GameObject::scaleMass(float scale) {
   return 1.0;
}

//------------------------------------Draw Stuff-------------------------------

vec3 GameObject::applyTransform(mat4 tran) {
   vec3 forceApplied;
   model.state.transform *= tran;
   return forceApplied;
}

//Re-calculate the transform matrix
void GameObject::updateTransformMatrix()
{
   model.state.transform = model.state.translate * model.state.rotation * model.state.scaling;
}

void GameObject::draw() {
   //glUseProgram(handles.ShadeProg);
   printf("%s:\n", name.c_str());
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

   // bind texture
   glActiveTexture(GL_TEXTURE0); //I dont know if this is necessary
   glBindTexture(GL_TEXTURE_2D, buffDat.texIndex);
   printf("texIndex: %i\n",buffDat.texIndex);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffDat.ibo);
   // draw!
   glDrawElements(GL_TRIANGLES, buffDat.numFaces, GL_UNSIGNED_INT, 0);
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
   moveBy(mov);
}
#endif
