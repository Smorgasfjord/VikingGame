//the apPosition of the vertex
attribute vec3 aPosition;

//the depth transforms
uniform mat4 uDepthMVP;

void main(){
	gl_Position = uDepthMVP * vec4(aPosition,1);
}

