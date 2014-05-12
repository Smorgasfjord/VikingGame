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
#include "Models/Model.h"
#include "Models/GameModel.h"

//Components
#include "Components/GameObject.hpp"
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


#define INIT_WIDTH 800
#define INIT_HEIGHT 600
#define pi 3.14159
#define PLANE_HEIGHT 1.25
#define WALL_COLLISION_SIZE .63

using namespace std;

//GL basics
int ShadeProg;
static float g_width, g_height;

//Handles to the shader data
GLHandles handles;

static const float g_groundY = 0;
static const float g_groundSize = 60.0;

//World
World world;
std::vector<int> platIdxs;

//Bjorn
GameModel bjornMod;
Bjorn bjorn;

//Hammer
int hammerTime;
GameModel hammerMod;
Hammer hammer;

//Text
int playerScore = 0;
double lastUpdated;
float frameRate;

//Light
glm::vec3 lightPos;

//Camera
float firstPersonHeight = 1.0f;
float camDistance = 4.0f;
glm::vec3 eye = glm::vec3(g_groundSize / 2.0f, firstPersonHeight, g_groundSize / 2.0);
glm::vec3 lookAt = glm::vec3(g_groundSize / 2.0f + 1.0f, firstPersonHeight, g_groundSize / 2.0 + 1.0);
glm::vec3 upV = glm::vec3(0.0, 1.0f, 0.0);

glm::mat4 ortho = glm::ortho(0.0f, (float)g_width,(float)g_height,0.0f, 0.1f, 100.0f);

//User interaction
glm::vec2 prevMouseLoc;

/* projection matrix */
void SetProjectionMatrix(bool drawText) {
   glm::mat4 Projection;
   if(!drawText)
      Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);
   else
      Projection = glm::ortho(0.0f, (float)g_width / 2,(float)g_height / 2,0.0f, 0.1f, 100.0f);
   safe_glUniformMatrix4fv(handles.uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls */
void SetView() {
   glm::mat4 view;
   view = glm::lookAt(eye, lookAt, upV);
   safe_glUniformMatrix4fv(handles.uViewMatrix, glm::value_ptr(view));
}

//Generates a random float within the range min-max
float randomFloat(float min, float max)
{
   return (float)((max - min) * (rand() / (double) RAND_MAX) + min);
}

int diffMs(timeval t1, timeval t2)
{
   return (((t1.tv_sec - t2.tv_sec) * 1000000) +
           (t1.tv_usec - t2.tv_usec))/1000;
}

//Resets bjorn to the start of the level, could be updated to add checkpoints
static void reset()
{
   eye = lookAt = world.getStart();
   eye.y += 1;
   eye.z -= camDistance;
   lookAt.y += .5;
   bjorn.setPos(lookAt);
   bjorn.setVelocity(glm::vec3(0));
}

/* Initialization of objects in the world. Only occurs Once */
void setWorld()
{
   //Mountain
   Model mountMod;
   Mountain mount;
   //Platforms
   GameModel platMod;
   std::vector<Platform> platforms;
   //Ground
   Model grndMod;
   std::vector<glm::vec3> groundTiles;
   
   //Initialize models
   grndMod = Model::init_Ground(g_groundY);
   mountMod = Model::init_Mountain();
   platMod = loadModel("Models/platform_2.dae", handles);
   bjornMod = loadModel("Models/bjorn_v1.1.dae", handles);
   hammerMod = loadModel("Models/bjorn_hammer.dae", handles);
   
   groundTiles.clear();
   for(int i = 0; i < g_groundSize; i++)
   {
      for(int j = 0; j < g_groundSize; j++)
         groundTiles.push_back(glm::vec3(i, g_groundY, j));
   }
   
   lightPos= glm::vec3(35, 15, -15);
   
   //Send light data to shader
   safe_glUniform3f(handles.uLightPos, lightPos.x, lightPos.y, lightPos.z);
   safe_glUniform3f(handles.uLightColor, 1, 1, 1);
   
   mount = Mountain(glm::vec3(g_groundSize / 2, 0, g_groundSize / 2), handles, mountMod);
   platforms = Platform::importLevel("mountain.lvl", handles, &platMod);
   cout << "Level loaded\n";
   world = World(platforms, mount, grndMod, &handles, ShadeProg);
   cout << "World worked\n";
   eye = lookAt = platforms[0].getPos();
   eye.y += 1;
   eye.z -= camDistance;
   lookAt.y += .5;
   /* This freezes when placing new platforms
   for (int i = 0; i < platforms.size(); i++) {
      platIdxs.push_back(world.placeObject(&(platforms[i]), &platMod));
   }
    */
   cout << "Platforms placed\n";
   bjorn = Bjorn(lookAt, handles, &bjornMod, world);
   cout << "Bjorn bound\n";
   hammer = Hammer("homar");
   hammer.initialize(&hammerMod, 0, 0, handles);
   hammer.setInWorld(world, &bjorn);
   hammerTime = world.placeObject(&hammer, &hammerMod);
   cout << "Hammer held\n";
   cout << "Lets play!\n";
   glfwSetTime(0);
   lastUpdated = glfwGetTime();
}

/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
   GLuint VS; //handles to shader object
   GLuint FS; //handles to frag shader object
   GLint vCompiled, fCompiled, linked; //status of shader
   
   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);
   
   //load the source
   glShaderSource(VS, 1, &vShaderName, NULL);
   glShaderSource(FS, 1, &fShaderName, NULL);
   
   //compile shader and print log
   glCompileShader(VS);
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
      return 0;
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
   handles.ShadeProg = ShadeProg;
   /* get handles to attribute and uniform data in shader */
   handles.aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
   handles.aNormal = safe_glGetAttribLocation(ShadeProg,	"aNormal");
   handles.aUV = safe_glGetAttribLocation(ShadeProg, "aUV");
   handles.uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");
   handles.depthBuff = safe_glGetUniformLocation(ShadeProg, "uDepthBuff");
   handles.depthMatrixID = safe_glGetUniformLocation(ShadeProg, "depthMVP");
   handles.uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   handles.uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   handles.uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   handles.uNormMatrix = safe_glGetUniformLocation(ShadeProg, "uNormMatrix");
   handles.uLightPos = safe_glGetUniformLocation(ShadeProg, "uLightPos");
   handles.uLightColor = safe_glGetUniformLocation(ShadeProg, "uLColor");
   handles.uEyePos = safe_glGetUniformLocation(ShadeProg, "uCamPos");
   handles.uMatAmb = safe_glGetUniformLocation(ShadeProg, "uMat.aColor");
   handles.uMatDif = safe_glGetUniformLocation(ShadeProg, "uMat.dColor");
   handles.uMatSpec = safe_glGetUniformLocation(ShadeProg, "uMat.sColor");
   handles.uMatShine = safe_glGetUniformLocation(ShadeProg, "uMat.shine");
   
   printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
}

/* helper function to set up material for shading */
void SetMaterial(int i) {
   
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

/* Some OpenGL initialization */
void Initialize ()
{
	// Start Of User Initialization
	glClearColor(.5, .5, .5, 1.0); // 1.0f, 1.0f, 1.0f, 1.0f);
	// Black Background
   //
 	glClearDepth (1.0f);	// Depth Buffer Setup
 	glDepthFunc (GL_LEQUAL);	// The Type Of Depth Testing
	glEnable (GL_DEPTH_TEST);// Enable Depth Testing
}

/* Main display function */
void Draw (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Start our shader
 	glUseProgram(ShadeProg);
   
   /* set up the projection and camera - do not change */
   SetProjectionMatrix(false);
   SetView();
   
   safe_glUniform3f(handles.uEyePos, eye.x, eye.y, eye.z);
   world.draw();
   bjorn.draw();
   hammer.draw();
	//Disable the shader
	glUseProgram(0);
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
   glm::vec2 currentPos = glm::vec2(p2wx(x), p2wy(y));
   currentPos.x += bjorn.getPos().x;
   currentPos.y += bjorn.getPos().y;
   
   hammer.updateAngle(currentPos.x, currentPos.y);
   
   prevMouseLoc = currentPos;
}

void mouseClick(GLFWwindow* window, int button, int action, int mods)
{
   if(action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1)
      hammer.flip();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   glm::vec3 delta;
   if(action == GLFW_PRESS || action == GLFW_REPEAT)
   {
      switch( key ) {
         case GLFW_KEY_D:
            bjorn.moveRight();
            break;
         case GLFW_KEY_A:
            bjorn.moveLeft();
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
      }
   }
}

void Animate()
{
   double curTime = glfwGetTime();
   static int wat = 0; //helps make updates only 30 times a second or so
   frameRate = 1000 / (curTime - lastUpdated + 1);
   /*
   cout << "Update @ " << curTime.tv_sec << "\n";
   cout << "\tCurrent score: " << playerScore << "\n";
   cout << "\tFramerate: " << frameRate << "\n";
    */
   //THESE HAVE TO STAY IN THIS ORDER
   bjorn.step();
   hammer.step();
  
   //kill bjorn if he's falling too fast
   if(bjorn.getVel().y < -10.0 && !DEBUG_GAME)
      reset();
   //updates the spatial data structure 
   if (wat % 10 == 0) {
      if (world.checkCollision(&hammer, hammerTime).obj.obj != hammerTime) {
         //printf("hammer hit the platform\n");
      }
      world.updateObject(&hammer, hammerTime);
   }
   wat++;
   eye = lookAt = bjorn.getPos();
   eye.y += 1;
   eye.z -= camDistance;
   lastUpdated = curTime;
}

int main( int argc, char *argv[] )
{
   GLFWwindow* window;
   
   g_width = INIT_WIDTH;
   g_height = INIT_HEIGHT;
   
   glfwSetErrorCallback(error_callback);
   
   if (!glfwInit())
      exit(EXIT_FAILURE);
   
   window = glfwCreateWindow(g_width, g_height, "Climb the Mountain!", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }
   
   glfwMakeContextCurrent(window);
   glewExperimental = GL_TRUE;
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
   }

   glfwSetKeyCallback(window, key_callback);
   glfwSetMouseButtonCallback(window, mouseClick);
   glfwSetCursorPosCallback(window, mouse);
   //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   glfwSetWindowSizeCallback(window, ReshapeGL);
   glfwSetWindowPos(window, 100, 100);
   
   unsigned int Error = glewInit();
   if (Error != GLEW_OK)
   {
	   std::cerr << "Error initializing glew! " << glewGetErrorString(Error) << std::endl;
	   system("PAUSE");
	   exit(33);
   }

   //test the openGL version
   getGLversion();
   //install the shader

   if (!InstallShader(textFileRead((char *)"Lab1_vert.glsl"), textFileRead((char *)"Lab1_frag.glsl"))) {
	   printf("Error installing shader!\n");
	   return 0;
   }

   Initialize();
   setWorld();
   
   while (!glfwWindowShouldClose(window))
   {
      Animate();
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
