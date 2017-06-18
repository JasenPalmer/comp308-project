#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>

#include "cgra_math.hpp"
#include "simplex_noise.hpp"
#include "opengl.hpp"
#include "simple_image.hpp"

using namespace std;
using namespace cgra;

SimplexNoise::SimplexNoise() {

}

SimplexNoise::~SimplexNoise() {
    
}

void SimplexNoise::init(int length, int width) {
    noise_length = length;
    noise_width = width;
}

void SimplexNoise::setSeed(int seed) {
    noise_seed = seed;
    srand(seed);
}

vector<vec3> SimplexNoise::generateVertices( float scale, int octaves, float persistence, float lacunarity, bool falloff) {
    use_falloff = falloff;
    vector<vec3> vertices;
    
    float maxHeight = -numeric_limits<float>::max();
    float minHeight = numeric_limits<float>::max();
    
    vector<vec2> octaveOffsets;
    
    generateFalloffMap();
    for (int i  = 0; i < octaves; i ++){
        float offsetX = randomFloat(-1000000, 1000000);
        float offsetY = randomFloat(-1000000, 1000000);
        octaveOffsets.push_back(vec2(offsetX, offsetY));
    }
    
    for (int z = 0; z < noise_length; z++) {
        //vector<float> row;
        for (int x = 0; x < noise_width; x++) {
            
            float frequency = 1;
            float amplitude = 1;
            float height = 0;
            for (int i =0; i < octaves; i++ ) {
                float sampleX = x / scale * frequency + octaveOffsets[i].x;
                float sampleZ = z / scale * frequency + octaveOffsets[i].y;
                
                float perlinValue = generateNoiseInternal(sampleX, sampleZ);
                height += perlinValue * amplitude;
                
                amplitude *= persistence;
                frequency *= lacunarity;
            }
            
            if (height > maxHeight) {
                maxHeight = height;
            } else if (height < minHeight) {
                minHeight = height;
            }
            vertices.push_back(vec3(x, height, z));
        }
    }

    cout << "Max Height: " << maxHeight << endl;
    cout << "Min Height: " << minHeight << endl;
    
    for (int i = 0; i < noise_length*noise_width; i++) {
        // Normalise the height
        float height = (vertices[i].y - minHeight) / (maxHeight - minHeight);
        if (use_falloff) {
            // If using falloff, subtract the falloff value from the height and reclamp.
            height = clamp(height - falloff_map[i], 1, 0);
        }
        vertices[i].y = height;
    }

    return vertices;
}

void SimplexNoise::setFalloff(bool falloff) {
    use_falloff = falloff;
}

void SimplexNoise::generateFalloffMap() {
    falloff_map.clear();
    cout << "Started: generating falloff map" << endl;
    for (int z = 0; z < noise_length; z++ ) {
        for (int x=0; x < noise_width; x++) {
            float zFrac = z / (float)noise_length * 2 -1;
            float xFrac = x / (float)noise_width * 2 -1;
            
            float value = max(abs(zFrac), abs(xFrac));
            value = falloffModifier(value);
            //cout << value << ", ";
            falloff_map.push_back(value);
        }
        //cout << endl;
    }
    cout << "Finished: generating falloff map" << endl;
}

// 2d Simplex Noise Implementation sourced from: http://webstaff.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
float SimplexNoise::generateNoiseInternal(float zin, float xin) {
    float n0, n1, n2; // Noise contributions from the three corners
    // Skew the input space to determine which simplex cell we're in
    float F2 = 0.5*(sqrt(3.0)-1.0);
    float s = (xin+zin)*F2; // Hairy factor for 2D
    
    int i = floor(xin+s);
    int j = floor(zin+s);
    
    float G2 = (3.0-sqrt(3.0))/6.0;
    float t = (i+j)*G2;
    float X0 = i-t; // Unskew the cell origin back to (x,y) space
    float Z0 = j-t;
    float x0 = xin-X0; // The x,y distances from the cell origin
    float z0 = zin-Z0;
    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if(x0>z0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)
    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    float z1 = z0 - j1 + G2;
    float x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
    float z2 = z0 - 1.0 + 2.0 * G2;
    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = perm(ii+perm(jj)) % 12;
    int gi1 = perm(ii+i1+perm(jj+j1)) % 12; int gi2 = perm(ii+1+perm(jj+1)) % 12;
    // Calculate the contribution from the three corners
    float t0 = 0.5 - x0*x0-z0*z0;
    if(t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, z0); // (x,y) of grad3 used for 2D gradient
    }
    
    float t1 = 0.5 - x1*x1-z1*z1;
    if(t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, z1);
    }
    
    float t2 = 0.5 - x2*x2-z2*z2;
    if(t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, z2);
    }
    
    // Add contributions from each corner to get the final noise value. // The result is scaled to return values in the interval [-1,1].
    return 70.0 * (n0 + n1 + n2);
}

float SimplexNoise::falloffModifier(float falloff) {
    // TODO: Modify falloff here
    float a = 4;
    float b = 2.2;
    return (pow(falloff, a)) / (pow(falloff, a) + pow((b - b*falloff), a));
}

float SimplexNoise::clamp(float value, float upper, float lower) {
    return min(upper, max(value, lower));
}
    
float SimplexNoise::dot(int g[], float x, float y) {
    return g[0]*x + g[1]*y;
}
    
int SimplexNoise::perm(int i) {
    return p[i & 255];
}
    
float SimplexNoise::randomFloat(float a,  float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}
