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
#define pi 3.14159
#define CAMERA_SPRING .15
#define CAM_Y_MAX_OFFSET 3
#define NUM_LIGHTS 5


using namespace std;

//GL basics
int mainDrawProg, depthBuffProg;
static float g_width, g_height;

GLuint fogTex;

//Handles to the shader data
GLHandles handles;

static const float g_groundSize = 60.0;

//World
World world;
std::vector<int> platIdxs;
GameModel simplePlatformMod;

//Bjorn
GameModel bjornMod;
Bjorn bjorn;
Transform_t bjornResetState;
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

glm::mat4 ortho = glm::ortho(0.0f, (float)g_width,(float)g_height,0.0f, 0.1f, 100.0f);

//User interaction
glm::vec2 prevMouseLoc;
glm::vec2 currentMouseLoc;

/* projection matrix */
void SetProjectionMatrix(bool drawText) {
   glm::mat4 Projection;
   if(!drawText)
      Projection = glm::perspective(90.0f, (float)g_width/g_height, 0.1f, 45.0f);
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
   return (int)(((t1.tv_sec - t2.tv_sec) * 1000000) +
           (t1.tv_usec - t2.tv_usec))/1000;
}

//Resets bjorn to the start of the level, or the last corner he rounded
static void reset()
{
   //lookAt.y += 1.0f;
   bjorn.facingRight = true; //THIS CAN BE WRONG
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
   skyBoxMod = loadModel("Models/SkyBox.dae", handles);
   mountMod = loadModel("Models/mountain.dae", handles);
   platMod = loadModel("Models/platform_2.dae", handles);
   hammerMod = loadModel("Models/bjorn_hammer.dae", handles);
   bjornMod = loadModel("Models/bjorn_v1.2.dae", handles);

   fogTex = LoadGLTextures("Models/FogTexture.png");
   glUniform1i(handles.uFogUnit, 1);
   simplePlatformMod = genSimpleModel(&platMod);
   
   for(int i = 0; i < NUM_LIGHTS; i++)
   {
      lightPos[i] = glm::vec3((10 * i) + 15, 10, -5);
   }
   
   //lightPos[0] = glm::vec3(50.188667, 2.512615, -0.142857);
   
   //Send light data to shader
   glUniform3fv(handles.uLightPos, NUM_LIGHTS, glm::value_ptr(lightPos[0]));
   //safe_glUniform3f(handles.uLightPos, lightPos[0].x, lightPos[0].y, lightPos[0].z);
   safe_glUniform3f(handles.uLightColor, 1, 1, 1);
   
   skyBox = GameObject("skybox");
   skyBox.initialize(skyBoxMod, 0, 4, handles);
   mount = Mountain(handles, &mountMod);
   platforms = Platform::importLevel("mountain.lvl", handles, &platMod);
   cout << "Level loaded\n";
   world = World(platforms, &simplePlatformMod, mount, &handles, mainDrawProg);
   cout << "World worked\n";
   //This stuff all assumes we start on the front of the mountain
   eye = lookAt = world.getStart();//glm::vec3(58, 15, 45);//world.getStart();
   eye.y += 1.0;
   eye.z -= camDistance;
   currentSide = MOUNT_FRONT;
   
   cout << "Platforms placed\n";
   bjorn = Bjorn(lookAt, handles, &bjornMod, &world);
   bjornResetState = bjorn.getState();
   cout << "Bjorn bound\n";
   hammer = Hammer("homar");
   hammer.setInWorld(&world, &bjorn, &hammerMod, handles);
   hammerResetState = hammer.getState();
   cout << "Hammer held\n";
   music.start();
   cout << "Lets play!\n";
   glfwSetTime(0);
   lastUpdated = glfwGetTime();
}

/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
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
   handles.uFogUnit = safe_glGetUniformLocation(ShadeProg, "uFogUnit");
   handles.depthBuff = safe_glGetUniformLocation(ShadeProg, "uDepthBuff");
   handles.depthMatrixID = safe_glGetUniformLocation(ShadeProg, "depthMVP");
   handles.uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   handles.uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   handles.uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   handles.uNormMatrix = safe_glGetUniformLocation(ShadeProg, "uNormMatrix");
   handles.uLightPos = safe_glGetUniformLocation(ShadeProg, "uLightPos");
   handles.uLightColor = safe_glGetUniformLocation(ShadeProg, "uLColor");
   handles.uEyePos = safe_glGetUniformLocation(ShadeProg, "uEyePos");
   handles.uMatAmb = safe_glGetUniformLocation(ShadeProg, "uMat.aColor");
   handles.uMatDif = safe_glGetUniformLocation(ShadeProg, "uMat.dColor");
   handles.uMatSpec = safe_glGetUniformLocation(ShadeProg, "uMat.sColor");
   handles.uMatShine = safe_glGetUniformLocation(ShadeProg, "uMat.shine");
   
   printf("sucessfully installed shader %d\n", ShadeProg);
   return ShadeProg;
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
   glEnable(GL_TEXTURE_2D);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

}

/* Main display function */
void Draw (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Start our shader
 	glUseProgram(mainDrawProg);
   
   /* set up the projection and camera - do not change */
   SetProjectionMatrix(false);
   SetView();
   glEnable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, fogTex);
   
   glDisable( GL_DEPTH_TEST );
   //skyBox.draw();
   glEnable( GL_DEPTH_TEST );
   safe_glUniform3f(handles.uEyePos, eye.x, eye.y, eye.z);
   world.draw(bjorn.mountainSide);
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
   glm::vec3 norm;

   frames++;
   if ((int)curTime > (int)lastUpdated) {
      frameRate = frames;
      frames = 0;
   }
   if (curTime - lastUpdated < 1.0/REFRESH_RATE) {
      return;
   }
   
   timeStep = curTime - lastUpdated;
   
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
   if(bjorn.getVel().y < (-3.0 * GRAVITY) && !DEBUG_GAME)
      Sound::scream();
   if(bjorn.getVel().y < (-4.0 * GRAVITY) && !DEBUG_GAME)
   {
      Sound::stopScream();
      reset();
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
      bjornResetState = bjorn.getState();
      hammerResetState = hammer.getState();
   }
   eye += ((bjorn.getPos() - norm * camDistance) - eye) * ((float)CAMERA_SPRING, 0.0f, (float)CAMERA_SPRING);
   
   skyBox.setPos(eye);
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
   
   //These may be mac only, not sure
   /*
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    */
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
   
   mainDrawProg = InstallShader(textFileRead((char *)"Shaders/Lab1_vert.glsl"), textFileRead((char *)"Shaders/Lab1_frag.glsl"));
   if (mainDrawProg == 0) {
	   printf("Error installing shader!\n");
	   return 0;
   }
   
   /* For loading a 2nd shader if/when we need it
   depthBuffProg = InstallShader(textFileRead((char *)"Shaders/DepthRTT_vert.glsl"), textFileRead((char *)"Shaders/DepthRTT_frag.glsl"));
   if (depthBuffProg == 0) {
	   printf("Error installing shader!\n");
	   return 0;
   }
    */
   
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
