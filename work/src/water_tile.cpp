#include <cmath>
#include <string>
#include <stdexcept>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"
#include "simple_image.hpp"
#include "simple_shader.hpp"

#include "water_tile.hpp"

using namespace std;
using namespace cgra;


Watertile::Watertile() {
	//set default values
	tilePosition = vec4(0.0, 2.0, 0.0, 0.0);
	viewpos = vec4(0.0, 10.0, 50.0, 0.0);
	screenDimension = vec2(640, 480);
	lightpos = vec4(0.0, 20.0, 20.0, 1.0);
	watercolor = vec4(0.0, 0.3, 0.5, 1.0);
	tileWidth = 10;
	distortAmount = 0.001;
	currentDistort = 1;

	//init shader/buffers/textures
	initialise();
}

Watertile::Watertile(vec4 pos, vec4 light, GLuint shader, int width) {
	tilePosition = pos;
	lightpos = light;
	waterShader = shader;
	tileWidth = width;

	viewpos = vec4(0.0, 10.0, 50.0, 0.0);
	screenDimension = vec2(1280, 720);
	watercolor = vec4(0.0, 0.3, 0.5, 1.0);
	distortAmount = 0.001;
	currentDistort = 1;

	//init shader/buffers/textures
	initialise();
}

Watertile::Watertile(vec4 pos, vec4 view, vec2 viewDimen, vec4 light, vec4 color, 
	int width, float speed) {
	// set tile values
	tilePosition = pos;
	viewpos = view;
	screenDimension = viewDimen;
	lightpos = light;
	watercolor = color;
	tileWidth = width;
	distortAmount = speed;
	currentDistort = 1;

	//init shader/buffers/textures
	initialise();
}

void Watertile::initialise() {
	// generate textures
	initialiseTextures();

	// load in the shader program
	initialiseShader();

	// create the buffers for reflection and refraction
	reflectionBuffer = createBuffer(reflectTexture, 0);
	refractionBuffer = createBuffer(refractTexture, depthMap);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Watertile::initialiseTextures() {
	// generate IDs for the textures
	glGenTextures(1, &reflectTexture);
	glGenTextures(1, &reflectTexture);
	glGenTextures(1, &refractTexture);
	glGenTextures(1, &depthMap);
	glGenTextures(1, &normalMap);
	glGenTextures(1, &dudvMap);

	// load the normal map and dudv map
	loadTexture("./work/res/textures/normal.png", normalMap);		
	loadTexture("./work/res/textures/waterDUDV.png", dudvMap);


}

void Watertile::loadTexture(string texturePath, GLuint texture) {
	// create image from texture filepath
	Image tex(texturePath);

	// bind texture as 2d texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Setup sampling strategies
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// fill texture with data from image
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tex.w, tex.h, tex.glFormat(), 
		GL_UNSIGNED_BYTE, tex.dataPointer());
}

void Watertile::initialiseShader() {
	if (waterShader == 0) {
		waterShader = makeShaderProgramFromFile({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
		{ "./work/res/shaders/waterShader.vert", "./work/res/shaders/waterShader.frag" });
	}
}

GLuint Watertile::createBuffer(GLuint texture, GLuint depthTexture) {
	GLuint buffer = 0;
	// generate buffer id and bind buffer
	glGenFramebuffers(1, &buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);

	glBindTexture(GL_TEXTURE_2D, texture);
	// create texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenDimension.x, screenDimension.y, 0, 
		GL_RGB, GL_UNSIGNED_BYTE, 0);
	// texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// depth for reflection
	if (depthTexture == 0) {
		GLuint depthrenderbuffer;
		glGenRenderbuffers(1, &depthrenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenDimension.x, 
			screenDimension.y);
		// set depthRenderBuffer as the depth component of the buffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 
			depthrenderbuffer);
	}
	// depth for refraction
	else {
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenDimension.x, 
			screenDimension.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// set depthTexture as the depth component of the buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	}
	// set texture as the color component of the buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	// add draw buiffers
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	GLenum err;
	if ((err = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		cout << err << " ERROR YO" << endl;
	}

	return buffer;
}

void Watertile::renderWater() {
	// enable flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	//glShadeModel(GL_SMOOTH);
	// use the water shader
	glUseProgram(waterShader);
	// set up all the uniform values
	glUniform1i(glGetUniformLocation(waterShader, "reflectionTexture"), 0);
	glUniform1i(glGetUniformLocation(waterShader, "refractionTexture"), 1);
	glUniform1i(glGetUniformLocation(waterShader, "normalMap"), 2);
	glUniform1i(glGetUniformLocation(waterShader, "dudvMap"), 3);
	glUniform1i(glGetUniformLocation(waterShader, "depthTexture"), 4);
	glUniform4f(glGetUniformLocation(waterShader, "viewpos"), viewpos.x, viewpos.y, viewpos.z, viewpos.w);
	glUniform4f(glGetUniformLocation(waterShader, "lightpos"), lightpos.x, lightpos.y, lightpos.z, lightpos.w);
	glUniform1f(glGetUniformLocation(waterShader, "distort"), currentDistort);
	glUniform1f(glGetUniformLocation(waterShader, "invDistort"), -currentDistort);
	glUniform4f(glGetUniformLocation(waterShader, "waterColor"), watercolor.r, watercolor.g, watercolor.b, watercolor.a);
	// increment the distortion
	currentDistort += distortAmount;

	// reflection texture in texture0
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, reflectTexture);
	// refraction texture in texture1
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, refractTexture);
	// normal in texture2
	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	// dudv in texture3
	glActiveTexture(GL_TEXTURE3);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, dudvMap);
	// depth in texutre4
	glActiveTexture(GL_TEXTURE4);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	float hWidth = float(tileWidth) / 2.0f;
	
	// draw the water quad 
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE0, 0.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE2, 0.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE3, 0.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE4, 0.0, 0.0);
	glVertex3f(tilePosition.x - hWidth, tilePosition.y, tilePosition.z - hWidth);	// top left
	glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);
	glMultiTexCoord2f(GL_TEXTURE1, 0.0, 1.0);
	glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);
	glMultiTexCoord2f(GL_TEXTURE3, 0.0, 1.0);
	glMultiTexCoord2f(GL_TEXTURE4, 0.0, 1.0);
	glVertex3f(tilePosition.x - hWidth, tilePosition.y, tilePosition.z + hWidth);	// bottom left
	glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0);
	glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
	glMultiTexCoord2f(GL_TEXTURE2, 1.0, 1.0);
	glMultiTexCoord2f(GL_TEXTURE3, 1.0, 1.0);
	glMultiTexCoord2f(GL_TEXTURE4, 1.0, 1.0);
	glVertex3f(tilePosition.x + hWidth, tilePosition.y, tilePosition.z + hWidth);	// bottom right
	glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE1, 1.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE2, 1.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE3, 1.0, 0.0);
	glMultiTexCoord2f(GL_TEXTURE4, 1.0, 0.0);
	glVertex3f(tilePosition.x + hWidth, tilePosition.y, tilePosition.z - hWidth);	// top right
	glEnd();
	glFlush();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);

	glUseProgram(0);
}

// getters

GLuint Watertile::getReflectionBuffer() {
	return reflectionBuffer;
}

GLuint Watertile::getRefractionBuffer() {
	return refractionBuffer;
}

GLuint Watertile::getReflectionTexture() {
	return reflectTexture;
}

GLuint Watertile::getRefractionTexture() {
	return refractTexture;
}

vec4 Watertile::getWaterPosition() {
	return tilePosition;
}



//setters

void Watertile::setLightPos(vec4 newPos) {
	lightpos = newPos;
}

void Watertile::setCameraPos(vec4 newPos) {
	viewpos = newPos;
}

void Watertile::setDistortAmount(float newSpeed) {
	distortAmount = newSpeed;
}

void Watertile::setWaterColor(vec4 newColor) {
	watercolor = newColor;
}

void Watertile::setReflectionBuffer(GLuint newBuffer) {
	reflectionBuffer = newBuffer;
}
