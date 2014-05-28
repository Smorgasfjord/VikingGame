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
uniform vec3 uCamPos;
uniform Material uMat;
uniform sampler2D uTexUnit;

void main() {
   vec4 texColor = texture2D(uTexUnit, vTexCoord);
   vec3 diffuse, specular, ambient;
   if (length(texColor.xyz) < 0.01) {
      texColor = vec4(1.0);
   }
   vec3 norm = normalize(vNorm) * (texColor.x + texColor.y + texColor.z) / 3.0;
   vec3 light = normalize(uLightPos);
   vec3 rVec = normalize(reflect(-light, norm));
   vec3 view = normalize(uCamPos - vPos);
   float maxCol = 1.0;
   diffuse = uLColor*normalize(max(dot(vNorm,light),0.0))*uMat.dColor;
   specular = uLColor*normalize(pow(max(dot(rVec,view),0.0),uMat.shine))*uMat.sColor;
   ambient = uLColor*uMat.aColor;
   vec3 phong = (diffuse + specular + ambient);
   if (phong.x > maxCol) maxCol = phong.x;
   if (phong.y > maxCol) maxCol = phong.y;
   if (phong.z > maxCol) maxCol = phong.z;
   phong = phong / maxCol;
   gl_FragColor = vec4(phong * texColor.xyz, 1.0);
}
