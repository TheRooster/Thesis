//Simple passthrough fragment shader for rectifying


varying vec3 vertexColor;


main(){
	gl_fragColor = vertexColor;
}