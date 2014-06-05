//#version 120
#define NUM_LIGHTS 5
#define NUM_LIGHTS_F 5.0
#define HEIGHT_TO_VIEW_DIST 50.0
#define VIEW_DIST 5.0//15.0 -  5.0
#define FOG_LINEAR 0.6 / VIEW_DIST
#define FOG_QUAD 0.4 / (VIEW_DIST * VIEW_DIST)
#define LIGHT_RADIUS 8.0
#define ATTENUATION_CONST 1.0
#define ATTENUATION_LINEAR .8 / LIGHT_RADIUS
#define ATTENUATION_QUADRATIC .6 / (LIGHT_RADIUS * LIGHT_RADIUS)

struct Material {
   vec3 aColor;
   vec3 dColor;
   vec3 sColor;
   float shine;
};

varying vec3 vPos;
varying vec3 vNorm;
varying vec2 vTexCoord;

uniform vec3 uLightPos[NUM_LIGHTS];
uniform vec3 uLColor;
uniform vec3 uEyePos;
uniform Material uMat;
uniform sampler2D uTexUnit;
uniform sampler2D uFogUnit;
uniform vec3 uWindVec;
uniform float uFogStrength;

void main() {
   vec4 texColor = texture2D(uTexUnit, vTexCoord), fogCol;
   vec3 diffuse, specular, ambient ,light, halfVec, preFog;
   vec3 lightSum = vec3(0.0);
   vec3 view = normalize(uEyePos - vPos);
   vec3 normEye = normalize(uEyePos), normPos = normalize(vPos);
   vec2 fogIdx = vec2((normEye.x + normEye.z + normPos.x + normPos.z + 4.0) * 0.125, (-normEye.y + normEye.z + 4.0 + -normPos.y + normPos.z) * 0.125);
   fogIdx += vec2((uWindVec.x + uWindVec.z + 2.0) * 0.25,(-uWindVec.y + uWindVec.z + 2.0) * 0.25);
   //while (fogIdx.x > 1.0) fogIdx.x -= 1.0;
   //while (fogIdx.y > 1.0) fogIdx.y -= 1.0;
   float attenuation, eyeDist, distance, intensity, maxCol = 1.0;
   //Fog stuff
   float viewDist = HEIGHT_TO_VIEW_DIST / uFogStrength;
   float fogLinear = 0.6 / viewDist;
   float fogQuad = 0.4 / (viewDist * viewDist);
   if (length(texColor.xyz) < 0.01) {
      texColor = vec4(1.0);
   }
   vec3 norm = normalize(vNorm * ((texColor.x + texColor.y + texColor.z) / 3.0));
   for(int i = 0; i < NUM_LIGHTS; i++)
   {
      light = normalize(uLightPos[i] - vPos);
      distance = length(uLightPos[i] - vPos);
      attenuation = 1.0 / (ATTENUATION_CONST + (ATTENUATION_LINEAR * distance) + (ATTENUATION_QUADRATIC * distance * distance));
      
      //Intensity of the diffuse light. Clamp within the 0-1 range.
      intensity = clamp(dot(norm, light), 0.0, 1.0);
      diffuse = intensity * vec3(1.0) * uMat.dColor;
      
      //Intensity of the specular light, Clamp within 0-1 range
      halfVec = normalize(light + view);
      intensity = pow(clamp(dot(normalize(vNorm), halfVec), 0.0, 1.0), uMat.shine);
      specular = intensity * uMat.sColor;
      lightSum += (diffuse + specular) * attenuation;
   }
   lightSum /= NUM_LIGHTS_F;
   ambient = vec3(1.0) * (length(uMat.aColor) > 0.01 ? uMat.aColor : vec3(0.1));
   vec3 phong = (lightSum + ambient);
   phong += ambient;
   if (phong.x > maxCol) maxCol = phong.x;
   if (phong.y > maxCol) maxCol = phong.y;
   if (phong.z > maxCol) maxCol = phong.z;
   phong = phong / maxCol;
   if (uMat.aColor.x == 1.0 && uMat.aColor.y == 1.0 && uMat.aColor.z == 1.0) {
      eyeDist = max(abs(uEyePos.x-vPos.x),max(abs(uEyePos.y-vPos.y),abs(uEyePos.z-vPos.z)));
   } else {
      eyeDist = length(uEyePos-vPos);
   }
   attenuation = clamp(eyeDist * eyeDist * fogQuad + eyeDist * fogLinear + (uFogStrength / 100.0), 0.0, 1.0);
   fogCol = texture2D(uFogUnit,fogIdx);
   gl_FragColor = vec4(phong * texColor.xyz * (1.0-attenuation) + fogCol.xyz * attenuation * 0.8, 1.0);
}
