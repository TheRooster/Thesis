#version 120
//Simple Texturing shader for rectifying


varying vec2 texCoord;

uniform sampler2D Tex1;

main(){

	gl_fragColor = texture(Tex1, texCoord);
}