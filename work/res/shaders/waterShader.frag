#version 120

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D normalMap;
uniform sampler2D dudvMap;
uniform sampler2D depthTexture;

uniform vec4 waterColor;

varying vec4 toLightV;		// vector to light 
varying vec4 firstDistort;	// first distort values
varying vec4 secondDistort;	// second distort values
varying vec4 clipSpace;		// clip space coords for projection
varying vec4 toViewV;		// vector to camera

void main(void) {
	// dudv scalars
	const vec4 sca = vec4(0.005, 0.005, 0.005, 0.005);
	const vec4 sca2 = vec4(0.02, 0.02, 0.02, 0.02);
	// tex coords scalar
	const vec4 tscale = vec4(0.25, 0.25, 0.25, 0.25);
	//
	const vec4 two = vec4(2.0, 2.0, 2.0, 1.0);
	const vec4 mone = vec4(-1.0, -1.0, -1.0, 1.0);
	const vec4 ofive = vec4(0.5,0.5,0.5,1.0);
	// specular exponent for specular highlight
	const float specExp = 64.0;
	// fog exponent (higher = less water fog)
	const float fogExp = 500;

	vec4 lightTS = normalize(toLightV);
	vec4 viewt = normalize(toViewV);
	
	vec4 disdis = texture2D(dudvMap, vec2(secondDistort * tscale));
	vec4 totalDist = texture2D(dudvMap, vec2(firstDistort + disdis*sca2));
	totalDist = totalDist * two + mone;
	totalDist = normalize(totalDist);
	totalDist *= sca;

	//load normalmap
	vec4 normal = texture2D(normalMap, vec2(firstDistort + disdis*sca2));
	normal = (normal-ofive) * two;
	normal = normalize(normal);

	//get projective texcoords
	vec4 tmp = vec4(1.0 / clipSpace.w);

	vec4 projCoord = clipSpace * tmp;
	projCoord += vec4(1.0);
	projCoord *= vec4(0.5);
	tmp = projCoord + totalDist;
	tmp = clamp(tmp, 0.001, 0.999);

	//load reflection,refraction and depth texture
	vec4 refl = texture2D(reflectionTexture, vec2(tmp));
	vec4 refr = texture2D(refractionTexture, vec2(tmp));
	vec4 wdepth = texture2D(depthTexture, vec2(tmp));

	wdepth = vec4(pow(wdepth.x, fogExp));
	vec4 invdepth = 1.0 - wdepth;

	//calculate specular highlight
	vec4 reflectLight = normalize(reflect(-lightTS, normal));
	float stemp = max(0.0, dot(viewt, reflectLight) );
	stemp = pow(stemp, specExp);
	vec4 specular = vec4(stemp);

	//calculate fresnel and inverted fresnel
	vec4 invfres = vec4( dot(normal, viewt) );
	vec4 fres = vec4(1.0) -invfres ;

	//calculate reflection and refraction
	refr *= invfres;
	refr *= invdepth;
	vec4 temp = waterColor * wdepth * invfres;
	refr += temp;
	refl *= fres;

	//add reflection and refraction
	tmp = refr + refl;

	gl_FragColor = tmp + specular;
}