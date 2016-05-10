//Simple passthrough fragment shader for rectifying


in vec3 vertexColor;


main(){
	gl_fragColor = vertexColor;
}