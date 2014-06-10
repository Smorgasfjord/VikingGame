//#version 120
#define NUM_LIGHTS 6
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
uniform vec3 uEyePos;
uniform vec3 uLightPos[NUM_LIGHTS];
uniform mat4 biasMatrix;

varying vec3 vPos;
varying vec3 vNorm;
varying vec3 vView;
varying vec2 vTexCoord;
varying vec4 vShadowCoord;
void main() {
   vec4 shadow;
   //Send position in model space
   vPos = (uModelMatrix * vec4(aPosition, 1.0)).xyz;
   //gl_Position is always projection space
   gl_Position = uProjMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
   
   //View
   vView = (uEyePos - vPos);
   //Shadow coordinates come from depthBias
   shadow = uDepthBiasMVP * vec4(aPosition,1);
   shadow /= shadow.w;
	vShadowCoord = biasMatrix * shadow;
   //Transform the normals
   vNorm = normalize((uNormMatrix * vec4(aNormal, 0.0)).xyz);
   vTexCoord = aUV;
}
