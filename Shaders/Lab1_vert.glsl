//#version 120
struct Material {
   vec3 aColor;
   vec3 dColor;
   vec3 sColor;
   float shine;
};

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aUV;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uNormMatrix;
uniform mat4 uDepthBiasMVP;

varying vec3 vPos;
varying vec3 vNorm;
varying vec2 vTexCoord;
varying vec4 vShadowCoord;
void main() {
   vec4 vPosition, transNorm;
   
   /* First model transforms */
   vPosition = uModelMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
   vPos = vec3(vPosition.x, vPosition.y, vPosition.z);
   vPosition = uViewMatrix * vPosition;
   gl_Position = uProjMatrix * vPosition;
   transNorm = uNormMatrix * vec4(aNormal.x, aNormal.y, aNormal.z, 0.0);
   
   vShadowCoord = uDepthBiasMVP * vec4(aPosition,1);
   
   vNorm = normalize(vec3(transNorm.x, transNorm.y, transNorm.z));
   vTexCoord = aUV;
}
