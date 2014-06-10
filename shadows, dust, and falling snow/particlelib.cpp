/*
   Update the forces on each particle
*/


//#include "final.h"



/*
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

      // Gravitation 
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


      // Viscous drag 
      p[i].force.x -= phys.viscousdrag * p[i].v.x;
      p[i].force.y -= phys.viscousdrag * p[i].v.y;
      p[i].force.z -= phys.viscousdrag * p[i].v.z;

   }
}

*/
/*
/*
   Perform one step of the solver
*/


/*
void UpdateParticles(
   PARTICLE *p,int np,
   PARTICLEPHYS phys,
   PARTICLESPRING *s,int ns,
   double dt,int method, int gravity)
{
   int i;
   PARTICLE *ptmp;
   PARTICLEDERIVATIVES *deriv;

   deriv = (PARTICLEDERIVATIVES *)malloc(np * sizeof(PARTICLEDERIVATIVES));

                                   // Midpoint 
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
         p[i].p.x += deriv[i].dpdt.x * dt;
         p[i].p.y += deriv[i].dpdt.y * dt;
         p[i].p.z += deriv[i].dpdt.z * dt;
         p[i].v.x += deriv[i].dvdt.x * dt;
         p[i].v.y += deriv[i].dvdt.y * dt;
         p[i].v.z += deriv[i].dvdt.z * dt;

         if(p[i].p.y < 0.0)
         {
            p[i].v.y = -p[i].v.y;
         }
         p[i].age += 1;
      }
      free(ptmp);

   free(deriv);
}
*/
/*
   Calculate the derivatives
   dp/dt = v
   dv/dt = f / m
*/
/*
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
*/
/*
   A 1st order 1D DE solver.
   Assumes the function is not time dependent.
   Parameters
      h      - step size
      y0     - last value
      method - algorithm to use [0,5]
      fcn    - evaluate the derivative of the field
*/
/*
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
*/
