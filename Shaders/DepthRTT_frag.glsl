void main(){
   //this is output to the texture (not rendered to screen)
   // so it makes sense that the output is just the depth
   gl_FragColor = vec4(gl_FragCoord.z, 0, 0, 1.0);
   gl_FragDepth = gl_FragCoord.z;
}
