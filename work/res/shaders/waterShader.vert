#version 120

varying vec4 toLightV;
varying vec4 firstDistort;
varying vec4 secondDistort;
varying vec4 clipSpace;
varying vec4 toViewV;
uniform vec4 viewpos, lightpos;
uniform float distort, invDistort;

void main(void) {

	vec4 temp;
	vec4 tangent = vec4(1.0, 0.0, 0.0, 0.0);
	vec4 norm = vec4(0.0, 1.0, 0.0, 0.0);
	vec4 binormal = vec4(0.0, 0.0, 1.0, 0.0);

	// view vector in tangent space
	temp = viewpos - gl_Vertex;
	toViewV.x = dot(temp, tangent);
	toViewV.y = dot(temp, binormal);
	toViewV.z = dot(temp, norm);
	toViewV.w = 1.0;

	//light vector in tangent space
	temp = lightpos - gl_Vertex;
	toLightV.x = dot(temp, tangent);
	toLightV.y = dot(temp, binormal);
	toLightV.z = dot(temp, norm);
	toLightV.w = 1.0;

	vec4 t1 = vec4(0.0, -distort, 0.0,0.0);
	vec4 t2 = vec4(0.0, -invDistort, 0.0,0.0);

	firstDistort = gl_MultiTexCoord0 + t1;
	secondDistort = gl_MultiTexCoord0 + t2;

	clipSpace = gl_ModelViewProjectionMatrix * gl_Vertex;;

	gl_Position = ftransform();
}