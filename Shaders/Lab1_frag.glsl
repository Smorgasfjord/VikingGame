//#version 120
#define FOG_TEX_SIZE 512.0
#define NUM_LIGHTS 5
#define NUM_LIGHTS_F 5.0
#define HEIGHT_TO_VIEW_DIST 100.0
#define LIGHT_RADIUS 15.0
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
varying vec4 vShadowCoord;
varying vec3 vView;

uniform vec3 uLightPos[NUM_LIGHTS];
uniform mat4 uViewMatrix;
uniform vec3 uEyePos;
uniform Material uMat;
uniform sampler2D uTexUnit;
uniform sampler2D uFogUnit;
uniform sampler2DShadow uShadowMap;
uniform vec3 uWindVec;
uniform float uFogStrength;

void main() {
   vec4 texColor = texture2D(uTexUnit, vTexCoord), fogCol;
   vec3 diffuse, specular, ambient, light, halfVec;
   vec3 lightSum = vec3(0.0);
   float attenuation, eyeDist, distance, intensity, maxCol = 1.0;
   //Fog stuff
   float viewDist = HEIGHT_TO_VIEW_DIST / uFogStrength;
   float fogLinear = 0.6 / viewDist;
   float fogQuad = 0.4 / (viewDist * viewDist);
   vec2 fogIdx = vec2((-uEyePos.x + uEyePos.z + 2.0 * gl_FragCoord.x/FOG_TEX_SIZE) * 0.2, (-uEyePos.y + 4.0 - 2.0 * gl_FragCoord.y/FOG_TEX_SIZE) * 0.2);
   fogIdx += vec2((uWindVec.x + uWindVec.z + 2.0) * 0.25,(-uWindVec.y + uWindVec.z + 2.0) * 0.25);
   //Shadow stuff
   float visibility, bias = 0.005;

   if (length(texColor.xyz) < 0.01) {
      texColor = vec4(1.0);
   }

   for(int i = 0; i < NUM_LIGHTS; i++)
   {
      light = normalize(uLightPos[i] - vPos);
      distance = length(uLightPos[i] - vPos);
      attenuation = 1.0 / (ATTENUATION_CONST + (ATTENUATION_LINEAR * distance) + (ATTENUATION_QUADRATIC * distance * distance));
      
      //Intensity of the diffuse light. Clamp within the 0-1 range.
      intensity = clamp(dot(light, vNorm), 0.0, 1.0);
      diffuse = intensity * uMat.dColor;
      
      //Intensity of the specular light, Clamp within 0-1 range
      halfVec = normalize(light + vView);
      intensity = pow(clamp(dot(vNorm, halfVec), 0.0, 1.0), uMat.shine);
      specular = intensity * uMat.sColor;
      
      lightSum += (diffuse + specular) * attenuation;
   }
   
   ambient = vec3(1.0) * (length(uMat.aColor) > 0.01 ? uMat.aColor : vec3(0.1));

   visibility = 1.0;

   //This should be in a loop with the poisson disk but glsl isn't happy with it
   visibility -= 0.2 * (1.0 - shadow2D(uShadowMap, vec3(vShadowCoord.xy + vec2( -0.94201624, -0.39906216 ) / 700.0,  vShadowCoord.z - bias)).z );
   visibility -= 0.2 * (1.0 - shadow2D(uShadowMap, vec3(vShadowCoord.xy + vec2( 0.94558609, -0.76890725 ) / 700.0,  vShadowCoord.z - bias)).z );
   visibility -= 0.2 * (1.0 - shadow2D(uShadowMap, vec3(vShadowCoord.xy + vec2( -0.094184101, -0.92938870 ) / 700.0,  vShadowCoord.z - bias)).z );
   visibility -= 0.2 * (1.0 - shadow2D(uShadowMap, vec3(vShadowCoord.xy + vec2( 0.34495938, 0.29387760 ) / 700.0,  vShadowCoord.z - bias)).z );

   //Calculate phong and move into 0-1 range
   vec3 phong = ((lightSum * visibility) + ambient);

   if (phong.x > maxCol) maxCol = phong.x;
   if (phong.y > maxCol) maxCol = phong.y;
   if (phong.z > maxCol) maxCol = phong.z;
   phong = phong / maxCol;
   
   //Fog
   if (uMat.aColor.x == 1.0 && uMat.aColor.y == 1.0 && uMat.aColor.z == 1.0) {
      eyeDist = max(abs(vView.x),max(abs(vView.y),abs(vView.z)));
   } else {
      eyeDist = length(vView);
   }
   attenuation = clamp(eyeDist * eyeDist * fogQuad + eyeDist * fogLinear + (uFogStrength / 100.0), 0.0, 1.0);
   fogCol = texture2D(uFogUnit,fogIdx);

   gl_FragColor = vec4(phong * texColor.xyz * (1.0-attenuation) + fogCol.xyz * attenuation * 0.8, 1.0);
   //gl_FragColor = vec4(phong, 1.0);
}
