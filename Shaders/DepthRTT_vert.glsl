#version 330 core

//the apPosition of the vertex
layout(location = 0) in vec3 aPos;

//the depth transforms
uniform mat4 depthMVP;

void main(){
	gl_Position =  depthMVP * vec4(aPos,1);
}

