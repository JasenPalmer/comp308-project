#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>

#include "cgra_math.hpp"
#include "terrain.hpp"
#include "opengl.hpp"
#include "simple_image.hpp"

using namespace std;
using namespace cgra;

SimplexNoise simplex_noise = SimplexNoise();

Terrain::Terrain(string textureFilename, int seed) {
    
    simplex_noise.setSeed(seed);
    t_texture_filename = textureFilename;
    
    terrain_length = 100;
    terrain_width = 100;
    
    x_off = (int)terrain_width/2;
    z_off = (int)terrain_length/2;
    
    y_off = -2;
    t_display_wire = false;
}

Terrain::~Terrain() {}

void Terrain::readTex(string filename) {
    Image tex(filename);
    
    glActiveTexture(GL_TEXTURE0); // Use slot 0, need to use GL_TEXTURE1 ... etc if using more than one texture PER OBJECT
    glGenTextures(1, &t_texture); // Generate texture ID
    glBindTexture(GL_TEXTURE_2D,  t_texture); // Bind it as a 2D texture
    
    // Setup sampling strategies
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Finnaly, actually fill the data into our texture
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tex.w, tex.h, tex.glFormat(), GL_UNSIGNED_BYTE, tex.dataPointer());
}

void Terrain::generateColors() {
    for (int z = 0; z < terrain_length; z++) {
        //terrain_normals[z] = new vec3 [terrain_width];
        vector<vec3> row;
        for (int x = 0; x < terrain_width; x ++) {
            float terrainHeight = terrain_heights[z][x];
            if (terrainHeight < 0.2) {
                //deep water
                row.push_back(vec3(0.05, 0.42, 0.71));
            } else if (terrainHeight < 0.4) {
                // shallow water
                row.push_back(vec3(0.2, 0.58, 0.82));
            } else if (terrainHeight < 0.45) {
                // sand
                row.push_back(vec3(0.96, 0.88, 0.47));
            } else if (terrainHeight < 0.65) {
                //grass
                row.push_back(vec3(0.57, 0.82, 0.2));
            } else if (terrainHeight < 0.9) {
                // rock
                row.push_back(vec3(0.36, 0.36, 0.36));
            } else {
                // snow
                row.push_back(vec3(1, 1, 1));
            }
            
        }
        terrain_colors.push_back(row);
    }

}

void Terrain::generateHeights() {
    cout << "Started: generating heights" << endl;
    terrain_heights = simplex_noise.generateNoiseMap(terrain_length, terrain_width, 30, 3, 0.5, 2);
    cout << "Finished: generating heights" << endl;
}

// Normal computation method sourced from: https://medium.com/@SoumitraSaxena/terrain-generation-from-a-heightmap-cccf50e961a9
void Terrain::generateNormals() {
    cout << "Started: generating normals" << endl;

    for (int z = 0; z < terrain_length; z++) {
        //terrain_normals[z] = new vec3 [terrain_width];
        vector<vec3> row;
        for (int x = 0; x < terrain_width; x ++) {
            vec3 sum(0.0f, 0.0f, 0.0f);
            
            vec3 out;
            if (z > 0)
            {
                out = vec3(0.0f, terrain_heights[z - 1][x] - terrain_heights[z][x], -1.0f);
            }
            vec3 in;
            if (z < terrain_length - 1)
            {
                in = vec3(0.0f, terrain_heights[z + 1][x] - terrain_heights[z][x], 1.0f);
            }
            vec3 left;
            if (x > 0)
            {
                left = vec3(-1.0f, terrain_heights[z][x - 1] - terrain_heights[z][x], 0.0f);
            }
            vec3 right;
            if (x < terrain_width - 1)
            {
                right = vec3(1.0f, terrain_heights[z][x + 1] - terrain_heights[z][x], 0.0f);
            }
            
            if (x > 0 && z > 0)
            {
                sum += normalize(cross(out, left));
            }
            if (x > 0 && z < terrain_length - 1)
            {
                sum += normalize(cross(left, in));
            }
            if (x < terrain_width - 1 && z < terrain_length - 1)
            {
                sum += normalize(cross(in, right));
            }
            if (x < terrain_width - 1 && z > 0)
            {
                sum += normalize(cross(right, out));
            }
            
            row.push_back(sum);
            
        }
        terrain_normals.push_back(row);
    }
    
    cout << "Finished: generating normal" << endl;
}

float Terrain::getHeight(int z, int x) {
    float height = terrain_heights[z][x];
    height = heightModifier(height);
    
    return height - y_off;
}

float Terrain::heightModifier(float height) {
    return exp(height*6-6) * height_multiplier;
}

void Terrain::createDisplayList() {
    
    cout << "Started: creating display list" << endl;
    if (t_displaylist) glDeleteLists(t_displaylist, 1);
    t_displaylist = glGenLists(1);
    
    glNewList(t_displaylist, GL_COMPILE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (int z = 0; z < terrain_length - 1; z++) { // Length
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x < terrain_width; x++) { // Width
            vec3 normal = terrain_normals[z][x];
            vec3 color = terrain_colors[z][x];
            glNormal3f(normal.x, normal.y, normal.z);
            glColor3f(color.r, color.g, color.b);
            glVertex3f(x-x_off, getHeight(z, x), z-z_off);
            
            normal = terrain_normals[z+1][x];
            color = terrain_colors[z+1][x];
            glNormal3f(normal.x, normal.y, normal.z);
            glColor3f(color.r, color.g, color.b);
            glVertex3f(x-x_off, getHeight(z+1, x), z+1-z_off);
        }
         glEnd();
    }
    glEndList();
    cout << "Finished: creating display list" << endl;
}

void Terrain::createDisplayListWire() {
    cout << "Started: creating display list" << endl;
    if (t_displaylist_wire) glDeleteLists(t_displaylist_wire, 1);
    t_displaylist_wire = glGenLists(1);
    
    glNewList(t_displaylist_wire, GL_COMPILE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for (int z = 0; z < terrain_length - 1; z++) { // Length
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x < terrain_width; x++) { // Width
            vec3 normal = terrain_normals[z][x];
            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(x-x_off, terrain_heights[z][x]*10-y_off, z-z_off);
            
            normal = terrain_normals[z+1][x];
            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(x-x_off, terrain_heights[z+1][x]*10-y_off, z+1-z_off); //TODO: Make these -10 offsets work for dynamic terrain sizes
        }
        glEnd();
    }
    glEndList();
    cout << "Finished: creating display list" << endl;
}

void Terrain::toggleWireMode() {
    t_display_wire = !t_display_wire;
    cout << "Wire mode state: " << t_display_wire << endl;
}

void Terrain::setupTerrain() {
    //readTex(t_texture_filename);
    generateHeights();
    generateNormals();
    generateColors();
    createDisplayList();
    createDisplayListWire();
}

void Terrain::renderTerrain() {
    glEnable(GL_COLOR_MATERIAL);
    GLuint displayList = t_display_wire ? t_displaylist_wire : t_displaylist;
    glCallList(displayList);
}
