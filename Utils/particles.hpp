#ifndef PARTICLES_HPP
#define PARTICLES_HPP

// Include standard headers
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>

#include <time.h>

// Include GLEW
#ifndef __unix__
#include <glew.h>
// Include GLFW
#include <glfw3.h>
#else 
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

// Include GLM
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"
#include "../glm/gtx/transform.hpp"

#include "GLHandles.h"
using namespace glm;

#define NUMBER_OF_DUST_PARTICLES 20
#define NUMBER_OF_SNOW_PARTICLES 200000
#define NUMBER_OF_SPRINGS 9
#define MAX_LIFESPAN 20


typedef struct {
   double mass;   /* Mass                          */
   double age, lifespan;   /* age & lifespan                  */
   vec3 p;      /* Position                      */
   vec3 v;      /* Velocity                      */
   vec3 force;      /* Force                         */
   int fixed;  /* Fixed point or free to move   */
   vec3 color; /*the particles color based on position */
} PARTICLE;

typedef struct {
   vec3 dpdt;
   vec3 dvdt;
} PARTICLEDERIVATIVES;

typedef struct {
   double gravitational;
   double viscousdrag;
} PARTICLEPHYS;

typedef struct {
   int from;
   int to;
   double springconstant;
   double dampingconstant;
   double restlength;
} PARTICLESPRING;

void CalculateForces(PARTICLE *,int,PARTICLEPHYS,PARTICLESPRING *,int, int);
void UpdateParticles(PARTICLE *,int,PARTICLEPHYS,PARTICLESPRING *,int,double,int, int);
void CalculateDerivatives(PARTICLE *,int,PARTICLEDERIVATIVES *);
double Solver1D(double,double,int,double (*)(double));

void initDustParticles();
void initDustParticles(float, float);

void initSnowParticles();
void initSnowParticles(float, float);


void moveParticles();
void drawParticles(GLHandles handles);


GLuint getDustBuff();
GLuint getSnowBuff();
GLuint getDustInxBuff();
GLuint getSnowInxBuff();



#endif
