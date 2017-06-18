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
    simplex_noise.init(terrain_length, terrain_width);
    simplex_noise.setSeed(seed);
    t_texture_filename = textureFilename;
    
    x_off = -(int)terrain_width/2;
    z_off = -(int)terrain_length/2;
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
    t_colors.clear();
    for (int i = 0; i < terrain_width * terrain_length; i++) {
        float terrainHeight = t_points[i].y;
        vec3 color;
        if (terrainHeight < 0.2) {
            //deep water
            color = vec3(0.05, 0.42, 0.71);
        } else if (terrainHeight < 0.4) {
            // shallow water
            color = vec3(0.2, 0.58, 0.82);
        } else if (terrainHeight < 0.5) {
            // sand
            color = vec3(0.96, 0.88, 0.47);
        } else if (terrainHeight < 0.65) {
            //grass
            color = vec3(0.57, 0.82, 0.2);
        } else if (terrainHeight < 0.9) {
            // rock
            color = vec3(0.36, 0.36, 0.36);
        } else {
            // snow
            color = vec3(1, 1, 1);
        }
        t_colors.push_back(color);
    }
}

void Terrain::generateHeights() {
    t_points.clear();
    cout << "Started: generating heights" << endl;
    t_points = simplex_noise.generateVertices(70, 3, 0.5, 2, true);
    cout << "Finished: generating heights" << endl;
}

void Terrain::generateUvs() {
    t_uvs.clear();
    
    for (int z = 0; z < terrain_length; z++) {
        for (int x = 0; x < terrain_width; x ++) {
            float x_uv = (float)x / (float)terrain_width;
            float z_uv = (float)z / (float)terrain_length;
            t_uvs.push_back(vec2(x_uv, z_uv));
        }
    }
}

// Normal computation method sourced from: https://medium.com/@SoumitraSaxena/terrain-generation-from-a-heightmap-cccf50e961a9
void Terrain::generateNormals() {
    t_normals.clear();
    cout << "Started: generating normals" << endl;
    vec3 normals[terrain_length][terrain_width];
    
    for (int z = 0; z < terrain_length; z++) {
        //terrain_normals[z] = new vec3 [terrain_width];
        vector<vec3> row;
        for (int x = 0; x < terrain_width; x ++) {
            vec3 sum(0.0f, 0.0f, 0.0f);
            
            vec3 out;
            if (z > 0)
            {
                out = vec3(0.0f, getHeight(z-1, x) - getHeight(z, x), -1.0f);
            }
            vec3 in;
            if (z < terrain_length - 1)
            {
                in = vec3(0.0f, getHeight(z+1, x) - getHeight(z, x), 1.0f);
            }
            vec3 left;
            if (x > 0)
            {
                left = vec3(-1.0f, getHeight(z, x-1) - getHeight(z, x), 0.0f);
            }
            vec3 right;
            if (x < terrain_width - 1)
            {
                right = vec3(1.0f, getHeight(z, x+1) - getHeight(z, x), 0.0f);
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
            
            normals[z][x] = sum;
            
        }
    }
    
    //Smooth out the normals
    const float FALLOUT_RATIO = 0.5f;
    for(int z = 0; z < terrain_length; z++)
    {
        for(int x = 0; x < terrain_width; x++)
        {
            vec3 sum = normals[z][x];
            
            if (x > 0)
            {
                sum += normals[z][x - 1] * FALLOUT_RATIO;
            }
            if (x < terrain_width - 1)
            {
                sum += normals[z][x + 1] * FALLOUT_RATIO;
            }
            if (z > 0)
            {
                sum += normals[z - 1][x] * FALLOUT_RATIO;
            }
            if (z < terrain_length - 1)
            {
                sum += normals[z + 1][x] * FALLOUT_RATIO;
            }
            
            if (length(sum) == 0)
            {
                sum = vec3(0.0f, 1.0f, 0.0f);
            }
            t_normals.push_back(normalize(sum));
        }
    }
    
    cout << "Finished: generating normals" << endl;
}

void Terrain::generateTriangles() {
    t_triangles.clear();
    cout << "Started: generating trinagles" << endl;
    for (int z = 0; z < terrain_length-1; z++) {
        for (int x = 0; x < terrain_width-1; x++) {
            
            int i1 = z * terrain_width + x;
            int i2 = (z+1) * terrain_width + x;
            int i3 = (z) * terrain_width + (x+1);
            int i4 = (z+1) * terrain_width + (x+1);
            
            // These normal indices are for per vertex normals, which I may yet use.
            vertex v1 = {i1, i1, i1};
            vertex v2 = {i2, i2, i2};
            vertex v3 = {i3, i3, i3};
            vertex v4 = {i4, i4, i4};
            
            triangle t1 = {{v1, v2, v3}};
            triangle t2 = {{v2, v3, v4}};
            
            t_triangles.push_back(t1);
            t_triangles.push_back(t2);
        }
    }
    cout << "Finished: generating trinagles" << endl;
}


float Terrain::getHeight(int z, int x) {
    vec3 p = t_points[z * terrain_width + x];
    return p.y;
}

float Terrain::heightModifier(float height) {
    height = exp(height*4-4) * height_multiplier;
    return height;
}

// Average the color of the vertices in the trinagle and returns a per triangle color.
vec3 Terrain::getTriangleColor(triangle t) {

    vec3 colors[3] = {t_colors[t.v[0].t], t_colors[t.v[1].t], t_colors[t.v[2].t]};
    
    if (colors[0] == colors[1]) {
        return colors[0];
    } else if (colors[0] == colors[2]) {
        return colors[0];
    } else if (colors[1] == colors[2]) {
        return colors[1];
    } else {
        int index = rand() % 3;
        return colors[index];
    }
}

void Terrain::createDisplayList() {
    cout << "Started: creating display list" << endl;
    if (t_displaylist) glDeleteLists(t_displaylist, 1);
    t_displaylist = glGenLists(1);

    glNewList(t_displaylist, GL_COMPILE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < t_triangles.size()-1; i += 2) {
        triangle t1 = t_triangles[i];
        vec3 color = getTriangleColor(t1);
        glColor3f(color.r, color.g, color.b);
        for (int k = i; k < i + 2; k++) {
            triangle t = t_triangles[k];
            for (int j = 0; j < 3; j++) {
                vertex v = t.v[j];
                vec3 point = t_points[v.p];
                vec3 normal = t_normals[v.n];
                vec2 uv = t_uvs[v.t];

                float height = heightModifier(point.y);
                
                glNormal3f(normal.x, normal.y, normal.z);
                glTexCoord2f(uv.x*100, uv.y*100);
                glVertex3f(point.x, height, point.z);
            }
        }

    }
    glEnd();
    glEndList();
    
    cout << "Finished: creating display list" << endl;
}

void Terrain::createDisplayListWire() {
    cout << "Started: creating display list" << endl;
    if (t_displaylist) glDeleteLists(t_displaylist_wire, 1);
    t_displaylist_wire = glGenLists(1);
    
    glNewList(t_displaylist_wire, GL_COMPILE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < t_triangles.size(); i ++) {
        triangle t = t_triangles[i];
        vec3 color = getTriangleColor(t);
        for (int j = 0; j < 3; j++) {
            vertex v = t.v[j];
            vec3 point = t_points[v.p];
            vec3 normal = t_normals[v.n];
            float height = heightModifier(point.y);
            
            glColor3f(color.r, color.g, color.b);
            glNormal3f(normal.x, normal.y, normal.z);
            
            glVertex3f(point.x, height, point.z);
        }
    }
    glEnd();
    glEndList();
    
    cout << "Finished: creating display list" << endl;
}

void Terrain::reseedTerrain(int seed) {
    simplex_noise.setSeed(seed);
    setupTerrain();
}

void Terrain::toggleWireMode() {
    t_display_wire = !t_display_wire;
    cout << "Wire mode state: " << t_display_wire << endl;
}

void Terrain::setupTerrain() {
    readTex(t_texture_filename);
    generateHeights();
    generateColors();
    generateUvs();
    generateTriangles();
    generateNormals();
    createDisplayList();
    createDisplayListWire();
}

void Terrain::renderTerrain() {
    //glEnable(GL_COLOR_MATERIAL);
    
    GLuint displayList = t_display_wire ? t_displaylist_wire : t_displaylist;
    glEnable(GL_COLOR_MATERIAL);
    //glEnable(GL_TEXTURE_2D);
    // Use Texture as the color
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    // Set the location for binding the texture
    glActiveTexture(GL_TEXTURE0);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, t_texture);
    
    glShadeModel(GL_SMOOTH);
    
    glPushMatrix();
    glTranslatef(x_off, y_off, z_off);
    glCallList(displayList);
    glPushMatrix();
}
