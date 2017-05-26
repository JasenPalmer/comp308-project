//---------------------------------------------------------------------------
//
// Copyright (c) 2016 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>

#include "cgra_math.hpp"
#include "geometry.hpp"
#include "opengl.hpp"
#include "simple_image.hpp"

using namespace std;
using namespace cgra;


Geometry::Geometry(string filename, string textureFilename, vec3 color, vec3 position, material material) {
	m_filename = filename;
    m_color = color;
    m_texture_filename = textureFilename;
    m_position = position;
    m_material = material;
    if (textureFilename != "") readTex(textureFilename);
	readOBJ(filename);
    
	if (m_triangles.size() > 0) {
		createDisplayListPoly();
		createDisplayListWire();
	}
}


Geometry::~Geometry() { }

void Geometry::readTex(string textureFilename) {
    Image tex(textureFilename);
    
    glActiveTexture(GL_TEXTURE0); // Use slot 0, need to use GL_TEXTURE1 ... etc if using more than one texture PER OBJECT
    glGenTextures(1, &m_texture); // Generate texture ID
    glBindTexture(GL_TEXTURE_2D,  m_texture); // Bind it as a 2D texture
    
    // Setup sampling strategies
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Finnaly, actually fill the data into our texture
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tex.w, tex.h, tex.glFormat(), GL_UNSIGNED_BYTE, tex.dataPointer());
}

void Geometry::readOBJ(string filename) {

	// Make sure our geometry information is cleared
	m_points.clear();
	m_uvs.clear();
	m_normals.clear();
	m_triangles.clear();

	// Load dummy points because OBJ indexing starts at 1 not 0
	m_points.push_back(vec3(0,0,0));
	m_uvs.push_back(vec2(0,0));
	m_normals.push_back(vec3(0,0,1));


	ifstream objFile(filename);

	if(!objFile.is_open()) {
		cerr << "Error reading " << filename << endl;
		throw runtime_error("Error :: could not open file.");
	}

	cout << "Reading file " << filename << endl;

	// good() means that failbit, badbit and eofbit are all not set
	while(objFile.good()) {

		// Pull out line from file
		string line;
		std::getline(objFile, line);
		istringstream objLine(line);

		// Pull out mode from line
		string mode;
		objLine >> mode;

		// Reading like this means whitespace at the start of the line is fine
		// attempting to read from an empty string/line will set the failbit
		if (!objLine.fail()) {

			if (mode == "v") {
				vec3 v;
				objLine >> v.x >> v.y >> v.z;
				m_points.push_back(v);

			} else if(mode == "vn") {
				vec3 vn;
				objLine >> vn.x >> vn.y >> vn.z;
				m_normals.push_back(vn);

			} else if(mode == "vt") {
				vec2 vt;
				objLine >> vt.x >> vt.y;
				m_uvs.push_back(vt);

			} else if(mode == "f") {

				vector<vertex> verts;
				while (objLine.good()){
					vertex v;

					//-------------------------------------------------------------
					// [Assignment 1] :
					// Modify the following to parse the bunny.obj. It has no uv
					// coordinates so each vertex for each face is in the format
					// v//vn instead of the usual v/vt/vn.
					//
					// Modify the following to parse the dragon.obj. It has no
					// normals or uv coordinates so the format for each vertex is
					// v instead of v/vt/vn or v//vn.
					//
					// Hint : Check if there is more than one uv or normal in
					// the uv or normal vector and then parse appropriately.
					//-------------------------------------------------------------

					// Assignment code (assumes you have all of v/vt/vn for each vertex)
                    
                    objLine >> v.p;
                    
                    if (objLine.peek() == '/') {
                        objLine.ignore(1);
                        
                        if (objLine.peek() == '/') {
                            objLine.ignore(1);
                            objLine >> v.n;
                        } else {
                            objLine >> v.t;
                            
                            if (objLine.peek() == '/') {
                                objLine.ignore(1);
                                objLine >> v.n;
                            }
                        }
                    }
                    
                    if (objLine.peek() == ' ') {
                        objLine.ignore(1);
                    }
                    

					verts.push_back(v);
				}

				// IFF we have 3 verticies, construct a triangle
				if(verts.size() >= 3){
					triangle tri;
					tri.v[0] = verts[0];
					tri.v[1] = verts[1];
					tri.v[2] = verts[2];
					m_triangles.push_back(tri);

				}
			}
		}
	}

	cout << "Reading OBJ file is DONE." << endl;
	cout << m_points.size()-1 << " points" << endl;
	cout << m_uvs.size()-1 << " uv coords" << endl;
	cout << m_normals.size()-1 << " normals" << endl;
	cout << m_triangles.size() << " faces" << endl;


	// If we didn't have any normals, create them
	if (m_normals.size() <= 1) createNormals();
    
    
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to populate the normals for
// the model currently loaded. Compute per face normals
// first and get that working before moving onto calculating
// per vertex normals.
//-------------------------------------------------------------
void Geometry::createNormals() {
	// YOUR CODE GOES HERE
    for (int i = 0; i < m_triangles.size(); i ++) {
        
        vec3 u = m_points[m_triangles[i].v[1].p] - m_points[m_triangles[i].v[0].p];
        vec3 v = m_points[m_triangles[i].v[2].p] - m_points[m_triangles[i].v[0].p];

        vec3 normal = normalize(cross(u, v));
//        normal.x = (u.y * v.z) - (u.z * v.y);
//        normal.y = (u.z * v.x) - (u.x * v.z);
//        normal.z = (u.x * v.y) - (u.y * v.x);
        
        m_normals.push_back(normal);
        m_triangles[i].v[0].n = i+1;
        m_triangles[i].v[1].n = i+1;
        m_triangles[i].v[2].n = i+1;

    }
    
    cout << m_normals.size()-1 << " normals after creating" << endl;
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to create display list
// of the obj file to show it as wireframe model
//-------------------------------------------------------------
void Geometry::createDisplayListPoly() {
	// Delete old list if there is one
	if (m_displayListPoly) glDeleteLists(m_displayListPoly, 1);

	// Create a new list
	cout << "Creating Poly Geometry" << endl;
	m_displayListPoly = glGenLists(1);
	glNewList(m_displayListPoly, GL_COMPILE);

	// YOUR CODE GOES HERE
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
	for (int i = 0; i < m_triangles.size(); i ++) {
		for (int j = 0; j < 3; j ++) {
			vertex v = m_triangles[i].v[j];
			vec3 v_point = m_points[v.p];
            vec3 v_normal = m_normals[v.n];
            
            glNormal3f(v_normal.x, v_normal.y, v_normal.z);
            glTexCoord2d(m_uvs[v.t].x * 4, m_uvs[v.t].y * 4);
            glVertex3f(v_point.x, v_point.y, v_point.z);
		}
	}
    
    glEnd();
	glEndList();
	cout << "Finished creating Poly Geometry" << endl;
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to create display list
// of the obj file to show it as polygon model
//-------------------------------------------------------------
void Geometry::createDisplayListWire() {
	// Delete old list if there is one
	if (m_displayListWire) glDeleteLists(m_displayListWire, 1);

	// Create a new list
	cout << "Creating Wire Geometry" << endl;
	m_displayListWire = glGenLists(1);
	glNewList(m_displayListWire, GL_COMPILE);

	// YOUR CODE GOES HERE
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_triangles.size(); i ++) {
        for (int j = 0; j < 3; j ++) {
            vertex v = m_triangles[i].v[j];
            vec3 v_point = m_points[v.p];
            vec3 v_normal = m_normals[v.n];
            
            glNormal3f(v_normal.x, v_normal.y, v_normal.z);
            glVertex3f(v_point.x, v_point.y, v_point.z);
        }
    }
    glEnd();
	glEndList();
	cout << "Finished creating Wire Geometry" << endl;
}


void Geometry::renderGeometry() {
	if (m_wireFrameOn) {

		//-------------------------------------------------------------
		// [Assignment 1] :
		// When moving on to displaying your obj, comment out the
		// wire_cow function & uncomment the glCallList function
		//-------------------------------------------------------------

		glShadeModel(GL_SMOOTH);
		glCallList(m_displayListWire);

	} else {

		//-------------------------------------------------------------
		// [Assignment 1] :
		// When moving on to displaying your obj, comment out the
		// cow function & uncomment the glCallList function
		//-------------------------------------------------------------
        // if m_texture_filename maybe...?
        if (m_texture_filename != "") {
            // Disable Drawing colours
            glDisable(GL_COLOR_MATERIAL);
            // Enable Drawing texures
            glEnable(GL_TEXTURE_2D);
            // Use Texture as the color
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            // Set the location for binding the texture
            glActiveTexture(GL_TEXTURE0);
            // Bind the texture
            glBindTexture(GL_TEXTURE_2D, m_texture);
        } else {
            // Disable Drawing textures
            glDisable(GL_TEXTURE_2D);
            // Enable Drawing colours
            glEnable(GL_COLOR_MATERIAL);
            glColor3f(m_color.r, m_color.g, m_color.b);
        }
        glMaterialfv(GL_FRONT, GL_AMBIENT, m_material.ambient.dataPointer());
        glMaterialfv(GL_FRONT, GL_SPECULAR, m_material.specular.dataPointer());
        glMaterialfv(GL_FRONT, GL_DIFFUSE, m_material.diffuse.dataPointer());
        glMaterialf(GL_FRONT, GL_SHININESS, m_material.shine * 128.0);
        
		glShadeModel(GL_SMOOTH);
        
        glPushMatrix();
        glTranslatef(m_position.x, m_position.y, m_position.z);
		glCallList(m_displayListPoly);
        glPopMatrix();

	}
}


void Geometry::toggleWireFrame() {
	m_wireFrameOn = !m_wireFrameOn;
}
