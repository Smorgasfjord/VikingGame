//#version 330 core
#define NUM_LIGHTS 5
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
uniform vec3 uLightPos[NUM_LIGHTS];
uniform vec3 uLColor;
uniform vec3 uEyePos;
uniform Material uMat;

uniform mat4 uDepthBuff;

varying vec3 vPos;
varying vec3 vNorm;
varying vec2 vTexCoord;
varying vec3 vView;
void main() {
   vec4 vPosition, transNorm;
   
   /* First model transforms */
   vPosition = uModelMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1);
   vPos = vec3(vPosition.x, vPosition.y, vPosition.z);
   vView = normalize(uEyePos - vPos);
   
   vPosition = uViewMatrix * vPosition;
   gl_Position = uProjMatrix * vPosition;
   transNorm = uNormMatrix * vec4(aNormal.x, aNormal.y, aNormal.z, 0.0);

   vNorm = normalize(vec3(transNorm.x, transNorm.y, transNorm.z));
   vTexCoord = aUV;
}