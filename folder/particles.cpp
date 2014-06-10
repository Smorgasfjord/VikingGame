
#include "particles.hpp"


void CalculateForces(
   PARTICLE *p,int np,
   PARTICLEPHYS phys,
   PARTICLESPRING *s,int ns, int gravity)
{
   int i,p1,p2;
   vec3 down = vec3(0.0,-1.0, 0.0);
   vec3 zero = vec3(0.0,0.0,0.0);
   vec3 f;
   double len,dx,dy,dz;

   for (i=0;i<np;i++) {
      p[i].force = zero;
      if (p[i].fixed)
         continue;

      /* Gravitation */
      if (gravity)
      {
         p[i].force.x += phys.gravitational * p[i].mass * down.x;
         p[i].force.y += phys.gravitational * p[i].mass * down.y;
         p[i].force.z += phys.gravitational * p[i].mass * down.z;
      }
      else
      {
         p[i].force.x -= phys.gravitational * p[i].mass * -1;
         p[i].force.y -= phys.gravitational * p[i].mass * down.y;
         p[i].force.z += phys.gravitational * p[i].mass * down.z;
      }


      /* Viscous drag */
      p[i].force.x -= phys.viscousdrag * p[i].v.x;
      p[i].force.y -= phys.viscousdrag * p[i].v.y;
      p[i].force.z -= phys.viscousdrag * p[i].v.z;

   }
}


/*
   Perform one step of the solver
*/
void UpdateParticles(
   PARTICLE *p,int np,
   PARTICLEPHYS phys,
   PARTICLESPRING *s,int ns,
   double dt,int method, int gravity)
{
//   printf("dt is (%lf)\n", dt);
   int i;
   PARTICLE *ptmp;
   PARTICLEDERIVATIVES *deriv;

   deriv = (PARTICLEDERIVATIVES *)malloc(np * sizeof(PARTICLEDERIVATIVES));

                                   /* Midpoint */
      CalculateForces(p,np,phys,s,ns, gravity);
      CalculateDerivatives(p,np,deriv);
      ptmp = (PARTICLE *)malloc(np * sizeof(PARTICLE));
      for (i=0;i<np;i++) {
         ptmp[i] = p[i];
         ptmp[i].p.x += deriv[i].dpdt.x * dt / 2;
         ptmp[i].p.y += deriv[i].dpdt.y * dt / 2;
         ptmp[i].p.z += deriv[i].dpdt.z * dt / 2;
         ptmp[i].p.x += deriv[i].dvdt.x * dt / 2;
         ptmp[i].p.y += deriv[i].dvdt.y * dt / 2;
         ptmp[i].p.z += deriv[i].dvdt.z * dt / 2;
      }
      CalculateForces(ptmp,np,phys,s,ns, gravity);
      CalculateDerivatives(ptmp,np,deriv);
      for (i=0;i<np;i++) {
//   printf("dt is (%lf)\n", dt);

//         printf("before p[%d].p.x = (%lf)\n", i, p[i].p.x);
         p[i].p.x += deriv[i].dpdt.x * dt;
//         printf("after p[%d].p.x = (%lf)\n", i, p[i].p.x);
         p[i].p.y += deriv[i].dpdt.y * dt;
         p[i].p.z += deriv[i].dpdt.z * dt;
         p[i].v.x += deriv[i].dvdt.x * dt;
         p[i].v.y += deriv[i].dvdt.y * dt;
         p[i].v.z += deriv[i].dvdt.z * dt;

//   printf("dt is (%lf)\n", dt);

//         printf("deriv[i].dvdt %d is at (%d, %d, %d) [%lf] \n", i, deriv[i].dvdt.x, deriv[i].dvdt.y, deriv[i].dvdt.z, dt);

         if(p[i].p.y < 0.0)
         {
            p[i].v.y = -p[i].v.y;
         }
         p[i].age += 1;

//         printf("particle %d is at (%lf, %lf, %lf)\n", i, p[i].p.x, p[i].p.y, p[i].p.z);
      }
      free(ptmp);

   free(deriv);
}

/*
   Calculate the derivatives
   dp/dt = v
   dv/dt = f / m
*/
void CalculateDerivatives(
   PARTICLE *p,int np,
   PARTICLEDERIVATIVES *deriv)
{
   int i;

   for (i=0;i<np;i++) {
      deriv[i].dpdt.x = p[i].v.x;
      deriv[i].dpdt.y = p[i].v.y;
      deriv[i].dpdt.z = p[i].v.z;
      deriv[i].dvdt.x = p[i].force.x / p[i].mass;
      deriv[i].dvdt.y = p[i].force.y / p[i].mass;
      deriv[i].dvdt.z = p[i].force.z / p[i].mass;
   }
}

/*
   A 1st order 1D DE solver.
   Assumes the function is not time dependent.
   Parameters
      h      - step size
      y0     - last value
      method - algorithm to use [0,5]
      fcn    - evaluate the derivative of the field
*/
double Solver1D(double h,double y0,int method,double (*fcn)(double))
{
   double ynew;
   double k1,k2,k3,k4,k5,k6;

   switch (method) {
   case 0:                          /* Euler method */
      k1 = h * (*fcn)(y0);
      ynew = y0 + k1;
      break;
   case 1:                          /* Modified Euler */
      k1 = h * (*fcn)(y0);
      k2 = h * (*fcn)(y0 + k1);
      ynew = y0 + (k1 + k2) / 2;
      break;
   case 2:                          /* Heuns method */
      k1 = h * (*fcn)(y0);
      k2 = h * (*fcn)(y0 + 2 * k1 / 3);
      ynew = y0 + k1 / 4 + 3 * k2 / 4;
      break;
   case 3:                          /* Midpoint */
      k1 = h * (*fcn)(y0);
      k2 = h * (*fcn)(y0 + k1 / 2);
      ynew = y0 + k2;
      break;
   case 4:                          /* 4'th order Runge-kutta */
      k1 = h * (*fcn)(y0);
      k2 = h * (*fcn)(y0 + k1/2);
      k3 = h * (*fcn)(y0 + k2/2);
      k4 = h * (*fcn)(y0 + k3);
      ynew = y0 + k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6;
      break;
   case 5:                          /* England 4'th order, six stage */
      k1 = h * (*fcn)(y0);
      k2 = h * (*fcn)(y0 + k1 / 2);
      k3 = h * (*fcn)(y0 + (k1 + k2) / 4);
      k4 = h * (*fcn)(y0 - k2 + 2 * k3);
      k5 = h * (*fcn)(y0 + (7 * k1 + 10 * k2 + k4) / 27);
      k6 = h * (*fcn)(y0 + (28*k1 - 125*k2 + 546*k3 + 54*k4 - 378*k5) / 625);
      ynew = y0 + k1 / 6 + 4 * k3 / 6 + k4 / 6;
      break;
   }

   return(ynew);
}


GLuint dustBuffObj, dustBuffObjTwo, dustIndxBuffObj;
GLuint snowBuffObj, snowBuffObjTwo, snowIndxBuffObj;

int dustParticles = 0, snowParticles = 0;
PARTICLE *dust, *snow;
int nsprings = 0;
PARTICLESPRING *springs;
PARTICLEPHYS physical;
float dustPos[NUMBER_OF_DUST_PARTICLES * sizeof(float)];
float snowPos[NUMBER_OF_SNOW_PARTICLES * sizeof(float)];

void createDustParticle(int index, float xPos, float yPos, float zPos)
{
   dust[index].p.x = xPos; 
   dust[index].p.z = zPos; 
   dust[index].p.y = yPos;

   dust[index].age = 0;
   dust[index].lifespan = 10*(rand()%MAX_LIFESPAN);
   dust[index].mass = .5;
   dust[index].fixed = false;
   dust[index].v.x = (rand() % 1000 - 500) /1000.0;
   dust[index].v.y = (rand() % 1000 - 500) /1000.0;
   dust[index].v.z = (rand() % 1000 - 500) /1000.0;

   dustPos[index*3] = dust[index].p.x;
   dustPos[index*3+1] = dust[index].p.y;
   dustPos[index*3+2] = dust[index].p.z;
}

void createDustParticle(int index)
{
   int radius = 1;
   float xPos, yPos, zPos;

   xPos = 1.0 * (rand() % 1000 - 500) /1000.0;
   zPos = 1.0 * (rand() % 1000 - 500) /1000.0;
   yPos = -.3 + sqrt(radius - zPos* zPos) -(xPos * xPos);

   createDustParticle(index, xPos, yPos, zPos);
}


void createSnowParticle(int index, float xPos, float yPos, float zPos)
{
   snow[index].p.x = xPos; 
   snow[index].p.z = zPos; 
   snow[index].p.y = yPos;

   snow[index].age = 0;
   snow[index].lifespan = 10000*(rand()%MAX_LIFESPAN);
   snow[index].mass = 1;
   snow[index].fixed = false;
   snow[index].v.x = (rand() % 1000 - 500) /1000.0;
   snow[index].v.y = (rand() % 1000 - 500) /1000.0;
   snow[index].v.z = (rand() % 1000 - 500) /1000.0;

   snowPos[index*3] = snow[index].p.x;
   snowPos[index*3+1] = snow[index].p.y;
   snowPos[index*3+2] = snow[index].p.z;
}

void createSnowParticle(int index)
{
   int radius = 1;
   float xPos, yPos, zPos;

   xPos = 10.0 * (rand() % 1000 - 500) /1000.0;
   zPos = 10.0 * (rand() % 1000 - 500) /1000.0;
   yPos = -.3 + sqrt(radius - zPos* zPos) -(xPos * xPos);

   createSnowParticle(index, xPos, yPos, zPos);
}


void initDustParticles() {
   initDustParticles(rand()%10, rand()%10);
}

void initDustParticles(float xPos, float yPos) {
   unsigned short dustIdx[NUMBER_OF_DUST_PARTICLES];
   int index;

   dustParticles = NUMBER_OF_DUST_PARTICLES;

   if (dust != NULL)
      free(dust);
   dust = (PARTICLE *)malloc(NUMBER_OF_DUST_PARTICLES * sizeof(PARTICLE));

   for (index=0;index<NUMBER_OF_DUST_PARTICLES;index++)
      createDustParticle(index, xPos, yPos, rand()%10);

   for(unsigned int i = 0; i< NUMBER_OF_DUST_PARTICLES; i++)
      dustIdx[i] = (short)i;

    glGenBuffers(1, &dustBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, dustBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dustPos), dustPos, GL_STATIC_DRAW);

    glGenBuffers(1, &dustIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dustIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(dustIdx), 
     dustIdx, GL_STATIC_DRAW);
}



void initSnowParticles() {
   initSnowParticles(rand(), rand());
}

void initSnowParticles(float xPos, float yPos) {
   unsigned short snowIdx[NUMBER_OF_SNOW_PARTICLES];
   int index;

   int maxPos = (int)(xPos > yPos ? xPos : yPos);

   snowParticles = NUMBER_OF_SNOW_PARTICLES;

   if (snow != NULL)
      free(snow);
   snow = (PARTICLE *)malloc(NUMBER_OF_SNOW_PARTICLES * sizeof(PARTICLE));

   for (index=0;index<NUMBER_OF_SNOW_PARTICLES;index++)
      createSnowParticle(index, xPos, yPos, rand()%maxPos);

   for(unsigned int i = 0; i< NUMBER_OF_SNOW_PARTICLES; i++)
      snowIdx[i] = (short)i;

    glGenBuffers(1, &snowBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, snowBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(snowPos), snowPos, GL_STATIC_DRAW);

    glGenBuffers(1, &snowIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, snowIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(snowIdx), 
     snowIdx, GL_STATIC_DRAW);
}


void moveParticles()
{

   double dt = .01;
//   printf("this is called at %lf\n", glfwGetTime());


   UpdateParticles(dust,dustParticles,physical,springs,nsprings,dt,1, 1);



   int dustPosNum = 0;
   int index = 0;

   for(index = 0; index < dustParticles; index++)
   {

      if(dust[index].age > dust[index].lifespan)
      {
         if(glfwGetTime() < 4)
         {
            createDustParticle(index);
//            printf("created a new particle at %lf\n", glfwGetTime());
         }
         else
            dustParticles--;
      }
      else
      {
         dustPos[index*3] = dust[index].p.x;
         dustPos[index*3+1] = dust[index].p.y;
         dustPos[index*3+2] = dust[index].p.z;
      }


   }

    glGenBuffers(1, &dustBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, dustBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dustPos), dustPos, GL_STATIC_DRAW);


   UpdateParticles(snow,snowParticles,physical,springs,nsprings,dt,1, 1);



   int snowPosNum = 0;

   for(index = 0; index < snowParticles; index++)
   {

      if(snow[index].age > snow[index].lifespan)
      {
            createSnowParticle(index);
      }
      else
      {
         snowPos[index*3] = snow[index].p.x;
         snowPos[index*3+1] = snow[index].p.y;
         snowPos[index*3+2] = snow[index].p.z;
      }


   }

    glGenBuffers(1, &snowBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, snowBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(snowPos), snowPos, GL_STATIC_DRAW);


}

GLuint getDustBuff()
{
return dustBuffObj;
}

GLuint getSnowBuff()
{
return snowBuffObj;
}

GLuint getDustInxBuff()
{
return dustIndxBuffObj;
}

GLuint getSnowInxBuff()
{
return snowIndxBuffObj;
}



