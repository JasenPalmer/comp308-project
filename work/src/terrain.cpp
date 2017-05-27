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
    readTex(textureFilename);
    generateTerrain();
    createNormals();
    createDisplayList();
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

void Terrain::generateTerrain() {


}

void Terrain::createNormals() {
    for (int i = 0; i < t_triangles.size(); i ++) {
        vec3 u = t_points[t_triangles[i].v[1].p] - t_points[t_triangles[i].v[0].p];
        vec3 v = t_points[t_triangles[i].v[2].p] - t_points[t_triangles[i].v[0].p];
        
        vec3 normal = normalize(cross(u, v));
        
        t_normals.push_back(normal);
        t_triangles[i].v[0].n = i+1;
        t_triangles[i].v[1].n = i+1;
        t_triangles[i].v[2].n = i+1;
    }
}

void Terrain::createDisplayList() {
    if (t_displayList) glDeleteLists(t_displayList, 1);
    
    t_displayList = glGenLists(1);
    glNewList(t_displayList, GL_COMPILE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < t_triangles.size(); i ++) {
        for (int j = 0; j < 3; j ++) {
            vertex v = t_triangles[i].v[j];
            vec3 v_point = t_points[v.p];
            vec3 v_normal = t_normals[v.n];
            
            glNormal3f(v_normal.x, v_normal.y, v_normal.z);
            glTexCoord2d(t_uvs[v.t].x * 4, t_uvs[v.t].y * 4);
            glVertex3f(v_point.x, v_point.y, v_point.z);
        }
    }
    glEnd();
    glEndList();
}

void Terrain::renderTerrain() {
    
}
