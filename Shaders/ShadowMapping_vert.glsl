#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

// Output data; varying per fragment
out vec3 vPos;
out vec3 vNorm;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out vec4 ShadowCoord;
out vec2 vTexCoord;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uLightPos;
uniform mat4 DepthBiasMVP;

void main(){
   mat4 MVP = uProjMatrix * uViewMatrix * uModelMatrix;
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(aPosition,1);
	
	ShadowCoord = DepthBiasMVP * vec4(aPosition,1);
	
	// Position of the vertex, in worldspace : M * position
	vPos = (uModelMatrix * vec4(aPosition,1)).xyz;
	
	// camera Vector, in camera space.
	// camera space makes the camera the origin (0,0,0) <thus the vec3(0,0,0)>
	EyeDirection_cameraspace = vec3(0,0,0) - ( uViewMatrix * uModelMatrix * vec4(aPosition,1)).xyz;

	// lightVector, in camera space
	LightDirection_cameraspace = (uViewMatrix * vec4(uLightPos,0)).xyz;
	
	vNorm = (uViewMatrix * uModelMatrix * vec4(aNormal,0)).xyz;
	
	// UV of the vertex. No special space for this one.
	vTexCoord = aUV;
}

