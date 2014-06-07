/*
 *  CPE 476 Lab 1
 *  Planes fly around an area and bounce off each other. You must run into them
 *  to make them crash.
 *
 *  Created by Taylor Woods on 4/10/14
 *
 *****************************************************************************/
#ifndef MAIN
#define MAIN

#ifdef __APPLE__
#include <glew.h>
#include <OPENGL/gl.h>
#include <sys/time.h>
#define GLFW_INCLUDE_GLU
#include <glfw3.h>
#endif
#ifdef __unix__
#include <GL/glew.h>
#include <GL/glut.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#endif
#ifdef _WIN32
#include <time.h>
#include <GL\glew.h>
#include <Windows.h>
#pragma comment (lib, "glfw3.lib")
#pragma comment (lib, "glew32s.lib")
#pragma comment (lib, "assimp.lib")
#pragma comment (lib, "DevIL.lib")
#endif

//Std
#include <stdlib.h>
#include <stdio.h>
#include <vector>

//Models
#include "Models/GameModel.h"

//Components
#include "Components/GameObject.h"
#include "Components/Platform.h"
#include "Components/Mountain.h"
#include "Components/Hammer.h"
#include "Components/Bjorn.h"
#include "Utils/World.h"

//Utils
#include "Utils/CMeshLoaderSimple.h"
#include "Utils/GLHandles.h"
#include "Utils/GLSL_helper.h"

//GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

//Audio
#include "Audio/Sound.h"
#include "Audio/Jukebox.h"

#define REFRESH_RATE 60.0
#define INIT_WIDTH 800
#define INIT_HEIGHT 600
#define FRAMEBUFFER_RES 1560
#define pi 3.14159
#define CAMERA_SPRING .15
#define CAM_Y_MAX_OFFSET 3
#define NUM_LIGHTS 5

using namespace std;

//GL basics
static float g_width, g_height;
GLuint shadowFrameBuffer;
GLuint shadowDepthTexture;

GLuint fogTex;
glm::vec3 wind = glm::vec3(0.0f);

//Handles to the shader data
GLHandles mainHandles;
GLHandles depthHandles;

static const float g_groundSize = 60.0;

//World
World world;
std::vector<int> platIdxs;
GameModel simplePlatformMod;

//Bjorn
GameModel bjornMod;
Bjorn bjorn;
Transform_t bjornResetState;
bool savedDirection = true;
static bool moveLeft = false, moveRight = false;

//Hammer
int hammerTime;
Transform_t hammerResetState;
GameModel hammerMod;
Hammer hammer;

GameObject skyBox;
GameModel skyBoxMod;

//Text
int playerScore = 0;
double lastUpdated = 0.0;
static float frameRate = 0.0;
static int frames = 0;

//Light
glm::vec3 lightPos[NUM_LIGHTS];

//Audio
Jukebox music;

//Camera
float camDistance = 4.0f;
glm::vec3 eye = glm::vec3(g_groundSize / 2.0f, 1.0, g_groundSize / 2.0);
glm::vec3 lookAt = glm::vec3(g_groundSize / 2.0f + 1.0f, 1.0, g_groundSize / 2.0 + 1.0);
glm::vec3 upV = glm::vec3(0.0, 1.0f, 0.0);
int currentSide;
float camYOffset = 0.0f;
bool manualCamControl = false;

//For shadow debugging
bool shadowMapDrawn = false;

//User interaction
glm::vec2 prevMouseLoc;
glm::vec2 currentMouseLoc;

//Functions
void setUpShadows();
void setUpMainDraw();
void shadow(GameObject *obj);

/* projection matrix */
void SetProjectionMatrix(bool drawText) {
   glm::mat4 Projection;
   if(!drawText)
      Projection = glm::perspective(90.0f, (float)g_width/g_height, 0.1f, 45.0f);
   else
      Projection = glm::ortho(0.0f, (float)g_width / 2,(float)g_height / 2,0.0f, 0.1f, 100.0f);
   safe_glUniformMatrix4fv(mainHandles.uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls */
void SetView() {
   glm::mat4 view;
   view = glm::lookAt(eye, lookAt, upV);
   safe_glUniformMatrix4fv(mainHandles.uViewMatrix, glm::value_ptr(view));
}

//Generates a random float within the range min-max
float randomFloat(float min, float max)
{
   return (float)((max - min) * (rand() / (double) RAND_MAX) + min);
}

int diffMs(timeval t1, timeval t2)
{
   return (int)(((t1.tv_sec - t2.tv_sec) * 1000000) +
           (t1.tv_usec - t2.tv_usec))/1000;
}

//Resets bjorn to the start of the level, or the last corner he rounded
static void reset()
{
   //lookAt.y += 1.0f;
   bjorn.facingRight = savedDirection; //THIS CAN BE WRONG
   hammer.setState(hammerResetState);
   hammer.setVelocity(glm::vec3(0));
   bjorn.setState(bjornResetState);
   bjorn.setVelocity(glm::vec3(0));
   bjorn.mountainSide = hammer.mountainSide = Mountain::getSide(bjorn.getPos());
   hammer.updateAngle(currentMouseLoc.x, currentMouseLoc.y-0.05f);
   hammer.updatePos(currentMouseLoc.x * camDistance, currentMouseLoc.y * camDistance);
   world.updateObject(&bjorn, bjorn.modelIdx);
   world.updateObject(&hammer, hammer.modelIdx);
   
   eye = lookAt = bjorn.getPos();
   eye.y += 1.5f;
   eye.z -= camDistance;
}

/* Initialization of objects in the world. Only occurs Once */
void setWorld()
{
   //Mountain
   GameModel mountMod;
   Mountain mount;
   //Platforms
   GameModel platMod;
   std::vector<Platform> platforms;
   
   //Initialize models
   
   glActiveTexture(GL_TEXTURE0);
   skyBoxMod = loadModel("Models/SkyBox.dae", mainHandles);
   mountMod = loadModel("Models/mountain.dae", mainHandles);
   platMod = loadModel("Models/platform_2.dae", mainHandles);
   hammerMod = loadModel("Models/bjorn_hammer.dae", mainHandles);
   bjornMod = loadModel("Models/bjorn_v1.3.dae", mainHandles);

   glActiveTexture(GL_TEXTURE1);
   fogTex = LoadGLTextures("Models/FogTexture.png");
   glUniform1i(mainHandles.uFogUnit, 1);
   simplePlatformMod = genSimpleModel(&platMod);
   
   for(int i = 0; i < NUM_LIGHTS; i++)
   {
      lightPos[i] = glm::vec3((10 * i) + 15, 10, -5);
   }
   //THIS LIGHT IS BEING USED FOR SHADOWING
   //lightPos[4] = glm::vec3(40, 10, -10);
   lightPos[4] = glm::vec3(42, 4, -7);
   //Send light data to shader
   glUniform3fv(mainHandles.uLightPos, NUM_LIGHTS, glm::value_ptr(lightPos[0]));

   safe_glUniform3f(mainHandles.uLightColor, 1, 1, 1);
   
   skyBox = GameObject("skybox");
   skyBox.initialize(skyBoxMod, 0, 4, mainHandles);
   mount = Mountain(mainHandles, &mountMod);

   platforms = Platform::importLevel("mountain.lvl", mainHandles, &platMod);
   cout << "Level loaded\n";
   world = World(platforms, &simplePlatformMod, mount, &mainHandles);
   cout << "World worked\n";
   //This stuff all assumes we start on the front of the mountain
   eye = lookAt = world.getStart();
   eye.y += 1.0;
   eye.z -= camDistance;
   currentSide = MOUNT_FRONT;
   skyBox.setPos(eye);
   
   cout << "Platforms placed\n";
   bjorn = Bjorn(lookAt, mainHandles, &bjornMod, &world);
   bjornResetState = bjorn.getState();
   cout << "Bjorn bound\n";
   hammer = Hammer("homar");
   hammer.setInWorld(&world, &bjorn, &hammerMod, mainHandles);
   hammerResetState = hammer.getState();
   cout << "Hammer held\n";
   music.start();
   
   cout << "Lets play!\n";
   glfwSetTime(0);
   lastUpdated = glfwGetTime();
}

/*function to help load the shaders (both vertex and fragment */
void InstallShader(const GLchar *vShaderName, const GLchar *fShaderName, GLHandles *handles) {
   GLuint VS; //handles to shader object
   GLuint FS; //handles to frag shader object
   GLint vCompiled, fCompiled, linked; //status of shader
   int ShadeProg;
   
   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);
   
   //load the source
   glShaderSource(VS, 1, &vShaderName, NULL);
   glShaderSource(FS, 1, &fShaderName, NULL);
   

   //compile shader and print log
   CheckedGLCall(glCompileShader(VS));
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
   printShaderInfoLog(VS);
   
   //compile shader and print log
   glCompileShader(FS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
   printShaderInfoLog(FS);
   
   if (!vCompiled || !fCompiled) {
      printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
      return ;
   }
   
   //create a program object and attach the compiled shader
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);
   
   glLinkProgram(ShadeProg);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   printProgramInfoLog(ShadeProg);
   
   glUseProgram(ShadeProg);
   handles->ShadeProg = ShadeProg;

   //Standard
   handles->aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
   handles->aNormal = safe_glGetAttribLocation(ShadeProg,	"aNormal");
   handles->uEyePos = safe_glGetUniformLocation(ShadeProg, "uEyePos");
   //Matrix Transforms
   handles->uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   handles->uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   handles->uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   handles->uNormMatrix = safe_glGetUniformLocation(ShadeProg, "uNormMatrix");
   //Texture
   handles->aUV = safe_glGetAttribLocation(ShadeProg, "aUV");
   handles->uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");
   //Light
   handles->uLightPos = safe_glGetUniformLocation(ShadeProg, "uLightPos");
   handles->uLightColor = safe_glGetUniformLocation(ShadeProg, "uLColor");
   //Material
   handles->uMatAmb = safe_glGetUniformLocation(ShadeProg, "uMat.aColor");
   handles->uMatDif = safe_glGetUniformLocation(ShadeProg, "uMat.dColor");
   handles->uMatSpec = safe_glGetUniformLocation(ShadeProg, "uMat.sColor");
   handles->uMatShine = safe_glGetUniformLocation(ShadeProg, "uMat.shine");
   //Fog
   handles->uFogUnit = safe_glGetUniformLocation(ShadeProg, "uFogUnit");
   handles->uFogStrength = safe_glGetUniformLocation(ShadeProg, "uFogStrength");
   handles->uWindVec = safe_glGetUniformLocation(ShadeProg, "uWindVec");
   //Shadows
   handles->depthMatrixID = safe_glGetUniformLocation(ShadeProg, "uDepthMVP");
   handles->shadowMapID = safe_glGetUniformLocation(ShadeProg, "uShadowMap");
   handles->depthBiasID = safe_glGetUniformLocation(ShadeProg, "uDepthBiasMVP");
   
   printf("sucessfully installed shader %d\n", ShadeProg);
}

/* Some OpenGL initialization */
void Initialize ()
{
	// Start Of User Initialization
	glClearColor(.5, .5, .5, 1.0); // 1.0f, 1.0f, 1.0f, 1.0f);
	// Black Background
   //
 	glClearDepth (1.0f);	// Depth Buffer Setup
 	glDepthFunc (GL_LESS);	// The Type Of Depth Testing
	glEnable (GL_DEPTH_TEST);// Enable Depth Testing
   glEnable(GL_TEXTURE_2D);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   
   //Shadow buffer
	shadowFrameBuffer = 0;
	glGenFramebuffers(1, &shadowFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
   
	// Depth texture
	CheckedGLCall(glGenTextures(1, &shadowDepthTexture));
	CheckedGLCall(glBindTexture(GL_TEXTURE_2D, shadowDepthTexture));
	CheckedGLCall(glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, FRAMEBUFFER_RES, FRAMEBUFFER_RES, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0));
	CheckedGLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	CheckedGLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	CheckedGLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	CheckedGLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	CheckedGLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
	CheckedGLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE));
   
   glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepthTexture, 0);
   
	CheckedGLCall(glDrawBuffer(GL_NONE));
   glReadBuffer(GL_NONE); //Thanks Katie Davis
   
	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   {
      cout << "Framebuffer incomplete\n";
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
      {
         cout << "Incomplete\n";
      }
      else if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
      {
         cout << "DIMENSIONS\n";
      }
      else if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
      {
         cout << "Missing attachment\n";
      }
      else
         cout << "Unsupported\n";
   }
}

/* Main display function */
void Draw (void)
{
   //This has to happen for the world to cull properly
   SetProjectionMatrix(false);
   SetView();
   std::vector<GameObject> drawnWorld = world.getDrawn(0);
   //Shadows
   setUpShadows();
   shadow(&bjorn);
   shadow(&hammer);
   for(int i = 0; i < drawnWorld.size(); i++)
   {
      shadow(&drawnWorld.at(i));
   }
   if(!shadowMapDrawn)
   {
      setUpMainDraw();
      safe_glUniform3f(mainHandles.uEyePos, eye.x, eye.y, eye.z);
      safe_glUniform3f(mainHandles.uWindVec, wind.x, wind.y, wind.z);
      
      safe_glUniform1f(mainHandles.uFogStrength, sqrt(bjorn.getPos().y) * 12.0f);
      
      glDisable( GL_DEPTH_TEST );
      skyBox.draw();
      glEnable( GL_DEPTH_TEST );
       
      safe_glUniform1f(mainHandles.uFogStrength, sqrt(bjorn.getPos().y));

      world.draw(bjorn.mountainSide);
      bjorn.draw();
      hammer.draw();
   }
	//Disable the shader
	glUseProgram(0);
}

//Sets up some OpenGL State for depth buffer, and starts the shader
void setUpShadows()
{
   if(!shadowMapDrawn)
   {
      glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
      glViewport(0,0,FRAMEBUFFER_RES,FRAMEBUFFER_RES); // Render on the whole framebuffer
   }
   else
   {
      //Use standard viewport and framebuffer
      glViewport(0,0,g_width,g_height);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }
   
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glUseProgram(depthHandles.ShadeProg);
}

void setUpMainDraw()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glViewport(0,0,g_width,g_height); // Render on the whole framebuffer
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glUseProgram(mainHandles.ShadeProg);
   glEnable(GL_TEXTURE_2D);
   
   //Pass depth texture
   glActiveTexture(GL_TEXTURE2); //PROBLEM AREA
   glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
   glUniform1i(mainHandles.shadowMapID, 2);
   
   //Pass Fog
   glActiveTexture(GL_TEXTURE1);
   glUniform1i(mainHandles.uFogUnit, 1);
   glBindTexture(GL_TEXTURE_2D, fogTex);
   glActiveTexture(GL_TEXTURE0);
   
   //set up the projection and camera - do not change
   SetProjectionMatrix(false);
   SetView();
   //glm::mat4 dViewMatrix = glm::lookAt(lightPos[4], lookAt, glm::vec3(0,1,0));//PROBLEM AREA
   //safe_glUniformMatrix4fv(mainHandles.uViewMatrix, glm::value_ptr(dViewMatrix));
}

void shadow(GameObject *obj)
{
   glm::mat4 dProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -1, 20);
   glm::mat4 dViewMatrix = glm::lookAt(lightPos[4], lookAt, glm::vec3(0,1,0));//PROBLEM AREA
   glm::mat4 depthMVP = dProjectionMatrix * dViewMatrix * obj->model.state.transform;
   
   // Send our transformation to the currently bound shader,
   // in the "MVP" uniform
   glUniformMatrix4fv(depthHandles.depthMatrixID, 1, GL_FALSE, glm::value_ptr(depthMVP));
   //All meshes
   for (int i = 0; i < obj->model.meshes.size(); i++) {
      // 1rst attribute buffer : vertices
      glEnableVertexAttribArray(depthHandles.aPosition);
      glBindBuffer(GL_ARRAY_BUFFER, obj->model.meshes[i].buffDat.vbo);
      glVertexAttribPointer(depthHandles.aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
      
      // Index buffer
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->model.meshes[i].buffDat.ibo);
      // Draw the shadow onto the frame
      glDrawElements(GL_TRIANGLES,obj->model.meshes[i].buffDat.numFaces, GL_UNSIGNED_SHORT, 0);
   }
   
   //All children, this is a little silly since it's so similar oh well
   for (int j = 0; j < obj->model.children.size(); j++) {
      for (int i = 0; i < obj->model.children[j].meshes.size(); i++) {
         // 1rst attribute buffer : position
         glEnableVertexAttribArray(depthHandles.aPosition);
         glBindBuffer(GL_ARRAY_BUFFER, obj->model.children[j].meshes[i].buffDat.vbo);
         glVertexAttribPointer(depthHandles.aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
         
         // Index buffer
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->model.children[j].meshes[i].buffDat.ibo);
         // Draw the shadow onto the frame
         
         glDrawElements(GL_TRIANGLES, obj->model.children[j].meshes[i].buffDat.numFaces, GL_UNSIGNED_SHORT, 0);
      }
   }
   
   glDisableVertexAttribArray(depthHandles.aPosition);
   //Store the depthMVP in this obj
   obj->setDepthMVP(depthMVP);
}

/* Reshape - note no scaling as perspective viewing*/
void ReshapeGL (GLFWwindow* window, int width, int height)
{
	g_width = (float)width;
	g_height = (float)height;
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));
}

float p2wx(double in_x) {
   if (g_width > g_height) {
      return g_width / g_height * (2.0 * in_x / g_width - 1.0);
   }
   else {
      return 2.0 * in_x / g_width - 1.0;
   }
}

float p2wy(double in_y) {
   //flip glut y
   in_y = g_height - in_y;
   if (g_width < g_height) {
      return g_height / g_width * (2.0 * in_y / g_height - 1.0);
   }
   else {
      return 2.0 * in_y / g_height - 1.0;
   }
}

static void error_callback(int error, const char* description)
{
   fputs(description, stderr);
}

/* Tracks mouse movement for the camera */
void mouse(GLFWwindow* window, double x, double y)
{
   currentMouseLoc = glm::vec2(p2wx(x), p2wy(y));
}

void mouseClick(GLFWwindow* window, int button, int action, int mods)
{
   if(action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
      hammer.flip();
   else if(action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT)
      hammer.manualLocked = true;
   else if(action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_RIGHT)
      hammer.manualLocked = false;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   glm::vec3 delta;
   if(action == GLFW_PRESS || action == GLFW_REPEAT)
   {
      switch( key ) {
         //Movement left/right
         case GLFW_KEY_D:
            moveRight = true;
            moveLeft = false;
            break;
         case GLFW_KEY_A:
            moveLeft = true;
            moveRight = false;
            break;
         //Camera control
         case GLFW_KEY_W:
            if(camYOffset < CAM_Y_MAX_OFFSET)
               camYOffset += 0.25;
            manualCamControl = true;
            break;
         case GLFW_KEY_S:
            if(camYOffset > -CAM_Y_MAX_OFFSET)
               camYOffset -= 0.25;
            manualCamControl = true;
            break;
         case GLFW_KEY_Q: case GLFW_KEY_ESCAPE:
            exit( EXIT_SUCCESS );
            break;
         case GLFW_KEY_SPACE:
            bjorn.jump();
            break;
         case GLFW_KEY_R:
            reset();
            break;
         case GLFW_KEY_EQUAL:
            music.volumeUp();
            break;
         case GLFW_KEY_MINUS:
            music.volumeDown();
            break;
         case GLFW_KEY_P:
            shadowMapDrawn = !shadowMapDrawn;
            break;
         case GLFW_KEY_LEFT:
            lightPos[4].x  += 1;
            glUniform3fv(mainHandles.uLightPos, NUM_LIGHTS, glm::value_ptr(lightPos[0]));
            break;
         case GLFW_KEY_RIGHT:
            lightPos[4].x -= 1;
            glUniform3fv(mainHandles.uLightPos, NUM_LIGHTS, glm::value_ptr(lightPos[0]));
            break;
         case GLFW_KEY_UP:
            lightPos[4].z += 1;
            glUniform3fv(mainHandles.uLightPos, NUM_LIGHTS, glm::value_ptr(lightPos[0]));
            break;
         case GLFW_KEY_DOWN:
            lightPos[4].z -= 1;
            glUniform3fv(mainHandles.uLightPos, NUM_LIGHTS, glm::value_ptr(lightPos[0]));
            break;
      }
   }
   if (action == GLFW_RELEASE) {
      switch( key ) {
         case GLFW_KEY_D:
            moveRight = false;
            break;
         case GLFW_KEY_A:
            moveLeft = false;
            break;
         case GLFW_KEY_W: case GLFW_KEY_S:
            manualCamControl = false;
            break;
         default:
            break;
      }
   }
}

void Animate()
{
   double curTime = glfwGetTime(), timeStep;
   //CollisionData dat;
   glm::vec3 norm, mWidth;

   frames++;
   if ((int)curTime > (int)lastUpdated) {
      frameRate = frames;
      frames = 0;
   }
   if (curTime - lastUpdated < 1.0/REFRESH_RATE) {
      return;
   }
   
   timeStep = curTime - lastUpdated;
   
   wind += glm::vec3(randomFloat(-0.01,0.01) + 0.05f, sqrt(fabsf(bjorn.getPos().y)) / 1000.0f, randomFloat(-0.005,0.005)) * (float)timeStep;

   hammer.updateAngle(currentMouseLoc.x, currentMouseLoc.y-0.05f);
   hammer.updatePos(currentMouseLoc.x * camDistance, currentMouseLoc.y * camDistance);
   prevMouseLoc = currentMouseLoc;
   if (moveLeft) {
      bjorn.moveLeft();
   }
   else if (moveRight) {
      bjorn.moveRight();
   }

   //THESE HAVE TO STAY IN THIS ORDER
   bjorn.step(timeStep);
   hammer.step(timeStep);
   bjorn.update(timeStep);
   hammer.update(timeStep);
   //updates the spatial data structure
   world.updateObject(&bjorn, bjorn.modelIdx);
   world.updateObject(&hammer, hammer.modelIdx);
  
   //kill bjorn if he's falling too fast
   if(bjorn.getVel().y < (-1.15 * GRAVITY))
   {
      Sound::scream();
      bjorn.screamed = true;
   }
   if(bjorn.getVel().y < (-2.0 * GRAVITY) && !DEBUG_GAME)
   {
      Sound::stopScream();
      reset();
   }
   
   //If bjorn is grounded allow him to scream again
   if(bjorn.getVel().y < .05 && bjorn.getVel().y > -.05)
   {
      bjorn.screamed = false;
      Sound::stopScream();
   }
   
   //Update camera
   lookAt = bjorn.getPos();

   //Get the normal to move the camera along
   Mountain::lockOn(bjorn.getPos(),norm);
   eye.y += CAMERA_SPRING * (bjorn.getPos().y - eye.y + 1.50f + camYOffset);
   if(!manualCamControl)
      camYOffset += CAMERA_SPRING * (bjorn.getPos().y - eye.y + 1.5f);
   
   //Mark a checkpoint if we're on a new side of the mountain and relatively steady in Y
   if(currentSide != bjorn.mountainSide && (bjorn.getVel().y < 0.2 && bjorn.getVel().y > -0.2))
   {
      currentSide = bjorn.mountainSide;
      //Update reset variables for checkpoint
      savedDirection = bjorn.facingRight;
      bjornResetState = bjorn.getState();
      hammerResetState = hammer.getState();
   }
   eye += ((bjorn.getPos() - norm * camDistance) - eye) * ((float)CAMERA_SPRING, 0.0f, (float)CAMERA_SPRING);
   mWidth = glm::vec3((float)MOUNT_WIDTH);
   skyBox.setPos(eye + (mWidth - lookAt * 2.0f) / mWidth * 0.5f);
   lastUpdated = curTime;
}

int main( int argc, char *argv[] )
{
   GLFWwindow* window;
   char title[256];
   
   g_width = INIT_WIDTH;
   g_height = INIT_HEIGHT;
   
   glfwSetErrorCallback(error_callback);
   
   if (!glfwInit())
      exit(EXIT_FAILURE);
   
   glfwWindowHint(GLFW_SAMPLES, 4);

   window = glfwCreateWindow(g_width, g_height, "Climb the Mountain!", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }
   
   CheckedGLCall(glfwMakeContextCurrent(window));
   glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

   CheckedGLCall(glfwSetKeyCallback(window, key_callback));
   glfwSetMouseButtonCallback(window, mouseClick);
   glfwSetCursorPosCallback(window, mouse);
   //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   glfwSetWindowSizeCallback(window, ReshapeGL);
   glfwSetWindowPos(window, 300, 300);
   
   int major;
   //test the openGL version
   CheckedGLCall(getGLversion());
   glGetIntegerv(GL_MAJOR_VERSION, &major);
   //install the shader
   
   InstallShader(textFileRead((char *)"Shaders/Lab1_vert.glsl"), textFileRead((char *)"Shaders/Lab1_frag.glsl"), &mainHandles);
   if (mainHandles.ShadeProg == 0) {
	   printf("Error installing shader!\n");
	   return 0;
   }

   // Load depth program
   InstallShader(textFileRead((char *)"Shaders/DepthRTT_vert.glsl"), textFileRead((char *)"Shaders/DepthRTT_frag.glsl"), &depthHandles);
   if (depthHandles.ShadeProg == 0) {
	   printf("Error installing shader!\n");
	   return 0;
   }

   Initialize();
   setWorld();
   
   while (!glfwWindowShouldClose(window))
   {
      Animate();
      sprintf(title, "Climb the Mountain! (%.1f %s)", frameRate, "FPS");
      glfwSetWindowTitle(window, title);

      Draw();
      glfwSwapBuffers(window);
      glfwPollEvents();
   }
   
   glfwDestroyWindow(window);
   
   glfwTerminate();
   exit(EXIT_SUCCESS);
   return 0;
}
#endif
