#version 220 core

struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

in vec3 vPos;
in vec3 vNorm;
in vec2 vTexCoord;

layout(location = 0) out vec3 color;

uniform vec3 uLightPos;
uniform vec3 uLColor;
uniform vec3 uCamPos;
uniform Material uMat;
uniform sampler2D uTexUnit;

void main() {
  vec3 texColor = texture(uTexUnit, vTexCoord).rgb;
  vec3 diffuse, specular, ambient;
  if (length(texColor.xyz) < 0.01) {
     texColor = vec3(1.0);
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
   
  color = phong * texColor;
}
