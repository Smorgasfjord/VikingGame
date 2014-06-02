#define NUM_LIGHTS 5
#define NUM_LIGHTS_F 5.0
#define LIGHT_RADIUS 15.0
#define VIEW_DIST 100.0 
#define ATTENUATION_CONST 0.9
#define ATTENUATION_LINEAR 0.8 / LIGHT_RADIUS
#define ATTENUATION_QUADRATIC 0.7 / (LIGHT_RADIUS * LIGHT_RADIUS)

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

void main() {
   vec4 texColor = texture2D(uTexUnit, vTexCoord), fogCol;
   vec3 diffuse, specular, ambient ,light, halfVec, preFog;
   vec3 lightSum = vec3(0.0);
   vec3 view = normalize(uEyePos - vPos);
   float attenuation, eyeDist, distance, intensity, maxCol = 1.0;
   fogCol = texture2D(uFogUnit,vTexCoord);
   if (length(texColor.xyz) < 0.01) {
      texColor = vec4(1.0);
   }
   vec3 norm = normalize(vNorm * ((texColor.x + texColor.y + texColor.z) / 3.0));
   for(int i = 0; i < NUM_LIGHTS; i++)
   {
      light = normalize(uLightPos[i] - vPos);
      distance = length(uLightPos[i] - vPos);
      eyeDist = length(vPos - uEyePos);
      attenuation = 1.0 / (ATTENUATION_CONST + (ATTENUATION_LINEAR * distance) + (ATTENUATION_QUADRATIC * distance * distance));
      
      //Intensity of the diffuse light. Clamp within the 0-1 range.
      intensity = clamp(dot(norm, light), 0.0, 1.0);
      diffuse = intensity * uLColor * uMat.dColor;
      
      //Intensity of the specular light, Clamp within 0-1 range
      halfVec = normalize(light + view);
      intensity = pow(clamp(dot(norm, halfVec), 0.0, 1.0), uMat.shine);
      specular = intensity * uLColor * uMat.sColor;
      preFog = (diffuse + specular) * attenuation;
      attenuation = max(eyeDist * eyeDist / VIEW_DIST, 1.0);
      lightSum += preFog;// * (1.0-attenuation) + fogCol.xyz * attenuation;
   }
   lightSum /= NUM_LIGHTS_F;
   ambient = uLColor * uMat.aColor;
   vec3 phong = (lightSum + ambient);
   if (phong.x > maxCol) maxCol = phong.x;
   if (phong.y > maxCol) maxCol = phong.y;
   if (phong.z > maxCol) maxCol = phong.z;
   phong = phong / maxCol;
   gl_FragColor = vec4(phong * texColor.xyz, 1.0);
}
