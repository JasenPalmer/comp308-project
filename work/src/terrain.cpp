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

Terrain::Terrain(string textureFilename) {
    t_texture_filename = textureFilename;
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

float Terrain::randomFloat(float a,  float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

// TODO: generate heights based on perlin noise funciton
void Terrain::generateHeights() {
    cout << "Started: generating heights" << endl;
    for (int z = 0;  z < TERRAIN_SIZE; z++) {
        for (int x = 0; x < TERRAIN_SIZE; x++) {
            terrain_heights[x][z] = randomFloat(-1.0f, 1.0f);
        }
    }
    cout << "Finished: generating heights" << endl;
}
// TODO: Find method of per vertice normal generation, may require heights to be genrated first.
void Terrain::generateNormals() {
    cout << "Started: generating normals" << endl;
    vec3 normals[TERRAIN_SIZE][TERRAIN_SIZE];
    for(int z = 0; z < TERRAIN_SIZE; z++)
    {
        for(int x = 0; x < TERRAIN_SIZE; x++)
        {
            vec3 sum(0.0f, 0.0f, 0.0f);
            
            vec3 out;
            if (z > 0)
            {
                out = vec3(0.0f, terrain_heights[z - 1][x] - terrain_heights[z][x], -1.0f);
            }
            vec3 in;
            if (z < TERRAIN_SIZE - 1)
            {
                in = vec3(0.0f, terrain_heights[z + 1][x] - terrain_heights[z][x], 1.0f);
            }
            vec3 left;
            if (x > 0)
            {
                left = vec3(-1.0f, terrain_heights[z][x - 1] - terrain_heights[z][x], 0.0f);
            }
            vec3 right;
            if (x < TERRAIN_SIZE - 1)
            {
                right = vec3(1.0f, terrain_heights[z][x + 1] - terrain_heights[z][x], 0.0f);
            }
            
            if (x > 0 && z > 0)
            {
                sum += normalize(cross(out, left));
            }
            if (x > 0 && z < TERRAIN_SIZE - 1)
            {
                sum += normalize(cross(left, in));
            }
            if (x < TERRAIN_SIZE - 1 && z < TERRAIN_SIZE - 1)
            {
                sum += normalize(cross(in, right));
            }
            if (x < TERRAIN_SIZE - 1 && z > 0)
            {
                sum += normalize(cross(right, out));
            }
            
            normals[z][x] = sum;
        }
    }
    
    //Smooth out the normals
    const float FALLOUT_RATIO = 0.5f;
    for(int z = 0; z < TERRAIN_SIZE; z++)
    {
        for(int x = 0; x < TERRAIN_SIZE; x++)
        {
            vec3 sum = normals[z][x];
            
            if (x > 0)
            {
                sum += normals[z][x - 1] * FALLOUT_RATIO;
            }
            if (x < TERRAIN_SIZE - 1)
            {
                sum += normals[z][x + 1] * FALLOUT_RATIO;
            }
            if (z > 0) 
            {
                sum += normals[z - 1][x] * FALLOUT_RATIO;
            }
            if (z < TERRAIN_SIZE - 1)
            {
                sum += normals[z + 1][x] * FALLOUT_RATIO;
            }
            
            if (length(sum) == 0)
            {
                sum = vec3(0.0f, 1.0f, 0.0f);
            }
            terrain_normals[z][x] = sum;
        }
    }
    cout << "Finished: generating normals" << endl;
}

void Terrain::createDisplayList() {
    cout << "Started: creating display list" << endl;
    if (t_displayList) glDeleteLists(t_displayList, 1);
    t_displayList = glGenLists(1);
    
    glNewList(t_displayList, GL_COMPILE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLE_STRIP);
    for (int z = 0; z < TERRAIN_SIZE - 1; z++) { // Length
        for (int x = 0; x < TERRAIN_SIZE; x++) { // Width
            vec3 normal = terrain_normals[x][z];
            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(x, terrain_heights[x][z], z);
            
            normal = terrain_normals[x][z+1];
            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(x, terrain_heights[x][z+1], z+1);
        }
    }
    glEnd();
    glEndList();
    cout << "Finished: creating display list" << endl;
}
void Terrain::setupTerrain() {
    //readTex(t_texture_filename);
    generateHeights();
    generateNormals();
    createDisplayList();
}

void Terrain::renderTerrain() {
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(0.0f, 1.0f,0.0f);
    glCallList(t_displayList);
}
