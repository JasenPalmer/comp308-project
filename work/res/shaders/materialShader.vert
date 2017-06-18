#version 120

varying vec3 vNormal;
varying vec3 vPosition;

void main() {

	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
	
	// Transform and pass on the normal/position to fragment shader
	vNormal = normalize(gl_NormalMatrix * gl_Normal);
	vPosition = vec3(gl_ModelViewMatrix * gl_Vertex);

	// IMPORTANT tell OpenGL where the vertex is
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

