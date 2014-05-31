//#version 330 core
#define NUM_LIGHTS 5
#define LIGHT_RADIUS 500.0
#define ATTENUATION_CONST 1.0
#define ATTENUATION_LINEAR 2.0 / LIGHT_RADIUS
#define ATTENUATION_QUADRATIC 1.0 / (LIGHT_RADIUS * LIGHT_RADIUS)

struct Material {
   vec3 aColor;
   vec3 dColor;
   vec3 sColor;
   float shine;
};

varying vec3 vPos;
varying vec3 vNorm;
varying vec2 vTexCoord;
varying vec3 vView;

uniform vec3 uLightPos[NUM_LIGHTS];
uniform vec3 uLColor;
uniform vec3 uEyePos;
uniform Material uMat;
uniform sampler2D uTexUnit;

void main() {
   vec4 texColor = texture2D(uTexUnit, vTexCoord);
   vec3 diffuse, specular, ambient, lightSum = vec3(0.0);
   if (length(texColor.xyz) < 0.01) {
      texColor = vec4(1.0);
   }
   vec3 norm = normalize(vNorm) * ((texColor.x + texColor.y + texColor.z) / 3.0);
   float maxCol = 1.0;
   float intensity, distance, attenuation;
   
   for(int i = 0; i < NUM_LIGHTS; i++)
   {
      vec3 light = normalize(uLightPos[i] - vPos);
      float distance = length(uLightPos[i] - vPos);
      attenuation = 1.0 / (ATTENUATION_CONST + (ATTENUATION_LINEAR * distance) + (ATTENUATION_QUADRATIC * distance * distance));
      //Diffuse
      intensity = clamp(dot(norm, light), 0.0, 1.0);
      diffuse = intensity * uLColor * uMat.dColor;
      
      //Specular
      vec3 halfVec = normalize(light + vView);
      intensity = pow(clamp(dot(norm, halfVec), 0.0, 1.0), uMat.shine);
      specular = intensity * uLColor * uMat.sColor;
      
      //Add specular and diffuse, and account for attenuation
      lightSum += (diffuse + specular);// * attenuation;
   }
   
   ambient = uLColor * uMat.aColor;
   vec3 phong = (lightSum + ambient);
   if (phong.x > maxCol) maxCol = phong.x;
   if (phong.y > maxCol) maxCol = phong.y;
   if (phong.z > maxCol) maxCol = phong.z;
   phong = phong / maxCol;
   //gl_FragColor = vec4(phong * texColor.xyz, 1.0);
   gl_FragColor = vec4(norm, 1.0);
   
}
