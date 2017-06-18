#pragma once

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"
#include "simplex_noise.hpp"

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
    int terrain_width = 240;
    int terrain_length = 240;
    
    int x_off;
    int z_off;
    int y_off;
    
    float height_multiplier = 12;
    
    bool t_display_wire;
    
    std::string t_texture_filename;     // String for storing texture filename
    GLuint t_texture;                   // ID of created texture
    std::vector<cgra::vec3> t_points;	// Point list
    std::vector<cgra::vec2> t_uvs;		// Texture Coordinate list
    std::vector<cgra::vec3> t_colors;   // Color Coordinate list
    std::vector<cgra::vec3> t_normals;	// Normal list
    std::vector<triangle> t_triangles;	// Triangle/Face list
    
    GLuint t_displaylist;       // ID for Polygon Displaylist
    GLuint t_displaylist_wire; // ID for Wire Dispalylist
    
    std::vector<std::vector<float>> terrain_heights;
    std::vector<std::vector<cgra::vec3>> terrain_normals;
    std::vector<std::vector<cgra::vec3>> terrain_colors;
    
    
    // Methods
    void readTex(std::string);
    void generateHeights();
    void generateNormals();
    void generateUvs();
    void generateColors();
    void generateTriangles();
    void createDisplayList();
    void createDisplayListWire();
    float getHeight(int, int);
    float heightModifier(float);
    cgra::vec3 getTriangleColor (triangle);
    
public:
    Terrain(std::string, int seed);
    ~Terrain();
    
    void reseedTerrain(int);
    void setupTerrain();
    void renderTerrain();
    void toggleWireMode();
    
};
