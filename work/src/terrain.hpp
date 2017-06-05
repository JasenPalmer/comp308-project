#pragma once

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"

struct vertex {
    int p = 0; // index for point in m_points
    int t = 0; // index for uv in m_uvs
    int n = 0; // index for normal in m_normals
};

struct triangle {
    vertex v[3]; //requires 3 verticies
};

int const TERRAIN_SIZE = 600;

class Terrain {
    
private:
    // Fields
    std::string t_texture_filename;     // String for storing texture filename
    GLuint t_texture;                   // ID of created texture
    std::vector<cgra::vec3> t_points;	// Point list
    std::vector<cgra::vec2> t_uvs;		// Texture Coordinate list
    std::vector<cgra::vec3> t_normals;	// Normal list
    std::vector<triangle> t_triangles;	// Triangle/Face list
    GLuint t_displayList;       // ID for Polygon Displaylist
    float terrain_heights[TERRAIN_SIZE][TERRAIN_SIZE];
    cgra::vec3 terrain_normals[TERRAIN_SIZE][TERRAIN_SIZE];
    
    // Methods
    void readTex(std::string);
    void generateHeights();
    void generateNormals();
    void createDisplayList();
    float randomFloat(float, float);
    
public:
    Terrain(std::string);
    ~Terrain();
    
    void setupTerrain();
    void renderTerrain();
    
};
