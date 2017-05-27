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
}

Terrain::~Terrain() {}

void Terrain::readTex(string filename) {
    
}

void Terrain::generateNormals() {
    
}

void Terrain::generateTerrain() {
    
}
