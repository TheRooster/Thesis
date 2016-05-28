#version 120
//"Vertex" shader to rectify the left and right image.
//takes as input the transformation matrix generated by stereoRectify()

uniform mat4 transformMatrix;
uniform mat4 projectionMatrix;

attribute vec2 vertexPosition;

varying vec2 texCoord;


main(){
	//put our 2 coordinates into a vec4 and then multiply by our rectification matrix.
	gl_position = vec4(vertexPosition, 1.0f, 1.0f) * transformMatrix * projectionMatrix;
}