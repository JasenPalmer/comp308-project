#pragma once

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"

class Watertile {
private:
	// width of tile
	int tileWidth;
	// tile position
	cgra::vec4 tilePosition;

	// camera position
	cgra::vec4 viewpos;
	// Dimensions of the viewPort
	cgra::vec2 screenDimension;
	// light position
	cgra::vec4 lightpos;
	// color of the water
	cgra::vec4 watercolor;
	// amount the distortion moves across the water tile
	float distortAmount;
	//currentDistortion
	float currentDistort;

	// reflection texture id
	GLuint reflectTexture;
	// refraction texture id
	GLuint refractTexture;
	// normal map id
	GLuint normalMap;
	// dudv map id
	GLuint dudvMap;
	// refraction depth map id
	GLuint depthMap;

	// reflection buffer id
	GLuint reflectionBuffer;
	// refraction buffer id
	GLuint refractionBuffer;
	
	// shader program id
	GLuint waterShader;

	GLuint createBuffer(GLuint, GLuint);
	void initialise();
	void initialiseTextures();
	void loadTexture(std::string, GLuint);
	void initialiseShader();

public:
	Watertile();
	Watertile(cgra::vec4, cgra::vec4, GLuint, int);

	// pos,view,screen,light,color,width,height,distort
	Watertile(cgra::vec4, cgra::vec4, cgra::vec2, cgra::vec4, cgra::vec4,
		int width = 10, float speed = 0.001);

	void renderWater();

	GLuint getReflectionBuffer();
	GLuint getRefractionBuffer();

	GLuint getReflectionTexture();
	GLuint getRefractionTexture();

	cgra::vec4 getWaterPosition();

	void setLightPos(cgra::vec4);
	void setCameraPos(cgra::vec4);
	void setWaterColor(cgra::vec4);
	void setDistortAmount(float);

	void setReflectionBuffer(GLuint);
};