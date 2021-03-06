#version 120

uniform float maxHeight;
uniform float minHeight;

varying vec3 vNormal;
varying vec3 vPosition;
varying float height;

void main() {
	
	vec3 L = normalize(gl_LightSource[0].position.xyz - vPosition);   
	vec3 E = normalize(-vPosition); // we are in Eye Coordinates, so EyePos is (0,0,0)  
	vec3 R = normalize(-reflect(L,vNormal));      

	// calculate Specular Term:
	vec4 Ispec = gl_FrontLightProduct[0].specular * pow(max(dot(R,E),0.0),0.3*3);
	Ispec = clamp(Ispec, 0.0, 1.0);
    
    float scaledHeight = ( (height-minHeight) / (maxHeight-minHeight) );
	// write Total Color:
    vec3 color; // Sand
    float lightFactor = dot(vNormal,L);

    lightFactor *= 2;
    
    if (scaledHeight < 0.1) {
        // sand
        color = vec3(0.96, 0.88, 0.47) * lightFactor;
    } else if (scaledHeight < 0.5) {
        //grass
        color = vec3(0.57, 0.82, 0.2) * lightFactor;
    } else if (scaledHeight < 0.9) {
        // rock
        color = vec3(0.36, 0.36, 0.36) * lightFactor;
    } else {
        // snow
        color = vec3(1, 1, 1) * lightFactor;
    }
    gl_FragColor = vec4(color, 1) + Ispec;
}
