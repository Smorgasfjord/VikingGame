#version 330 core

//the apPosition of the vertex
layout(location = 0) in vec3 vertexPosition_modelspace;

//the depth transforms
uniform mat4 depthMVP;

void main(){
	gl_Position =  depthMVP * vec4(vertexPosition_modelspace,1);
}
