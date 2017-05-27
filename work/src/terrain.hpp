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

class Terrain {
    
private:
    // Fields
    int const TERRAIN_SIZE = 600;
    
    std::string t_texture_filename;     // String for storing texture filename
    GLuint t_texture;                   // ID of created texture
    std::vector<cgra::vec3> t_points;	// Point list
    std::vector<cgra::vec2> t_uvs;		// Texture Coordinate list
    std::vector<cgra::vec3> t_normals;	// Normal list
    std::vector<triangle> t_triangles;	// Triangle/Face list
    GLuint t_displayList = 0;       // ID for Polygon Displaylist
    
    // Methods
    void readTex(std::string);
    void generateTerrain();
    void createNormals();
    void createDisplayList();
    
public:
    Terrain(std::string);
    ~Terrain();
    
    void renderTerrain();
    
};
