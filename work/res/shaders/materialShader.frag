#version 120

varying vec3 vNormal;
varying vec3 vPosition; 

void main() {
	
	vec3 L = normalize(gl_LightSource[0].position.xyz - vPosition);   
	vec3 E = normalize(-vPosition); // we are in Eye Coordinates, so EyePos is (0,0,0)  
	vec3 R = normalize(-reflect(L,vNormal));  

	//calculate Ambient Term:  
	vec4 Iamb = gl_FrontLightProduct[0].ambient; // + gl_FrontMaterial.ambient  

	//calculate Diffuse Term:  
	vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(vNormal,L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0);     

	// calculate Specular Term:
	vec4 Ispec = gl_FrontLightProduct[0].specular 
	            * pow(max(dot(R,E),0.0),0.3*gl_FrontMaterial.shininess);
	Ispec = clamp(Ispec, 0.0, 1.0); 
	// write Total Color:  
	gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec; 
}