//#version 330 core

struct Material {
   vec3 aColor;
   vec3 dColor;
   vec3 sColor;
   float shine;
};

varying vec3 vPos;
varying vec3 vNorm;
varying vec2 vTexCoord;

uniform vec3 uLightPos;
uniform vec3 uLColor;
uniform vec3 uEyePos;
uniform Material uMat;
uniform sampler2D uTexUnit;

void main() {
   vec4 texColor = texture2D(uTexUnit, vTexCoord);
   vec3 diffuse, specular, ambient;
   if (length(texColor.xyz) < 0.01) {
      texColor = vec4(1.0);
   }
   vec3 norm = normalize(vNorm) * ((texColor.x + texColor.y + texColor.z) / 3.0);
   vec3 light = uLightPos - vPos;
   float lightDist = length(light); //get the distance from the light
   lightDist *= lightDist; //Square the distance not currently used, we don't have enough light
   light = normalize(light); //Normalize the light
   vec3 view = normalize(uEyePos - vPos);
   float maxCol = 1.0;
   
   //Intensity of the diffuse light. Clamp within the 0-1 range.
   float intensity = clamp(dot(norm, light), 0.0, 1.0);
   diffuse = intensity * uLColor * uMat.dColor; // lightDist;
   
   //Intensity of the specular light, Clamp within 0-1 range
   vec3 halfVec = normalize(light + view);
   intensity = pow(clamp(dot(norm, halfVec), 0.0, 1.0), uMat.shine);
   specular = intensity * uMat.sColor; // lightDist;
   
   ambient = uLColor * uMat.aColor;
   vec3 phong = (diffuse + specular + ambient);
   if (phong.x > maxCol) maxCol = phong.x;
   if (phong.y > maxCol) maxCol = phong.y;
   if (phong.z > maxCol) maxCol = phong.z;
   phong = phong / maxCol;
   gl_FragColor = vec4(phong * texColor.xyz, 1.0);
}
