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
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>

#include "cgra_geometry.hpp"
#include "cgra_math.hpp"
#include "simple_image.hpp"
#include "simple_shader.hpp"
#include "opengl.hpp"
#include "terrain.hpp"
#include "water_tile.hpp"

using namespace std;
using namespace cgra;

// Window
//
GLFWwindow* g_window;


int base_seed = 1497779637;
Terrain terrain = Terrain("./work/res/textures/grass.jpg", base_seed); // Maybe set this seed based on a ui field if I have time.


// Projection values
// 
float g_fovy = 20.0;
float g_znear = 0.1;
float g_zfar = 1000.0;


// Mouse controlled Camera values
//
bool g_leftMouseDown = false;
vec2 g_mousePosition;
float g_pitch = 15;
float g_yaw = -45;
float g_zoom = 1.0;

//camera position
//
float distToCamera = 150.0f;
vec4 g_camera_position = vec4(0.0, 0.0, 25.0, 1.0);
vec3 g_camera_direction = vec3(0.0, 0.0, 0.0);
vec3 g_camera_up = vec3(0.0, 1.0, 0.0);

// light position
//
vec4 g_light_pos = vec4(-100.0, 50.0, -100.0, 1.0);

// Values and fields to showcase the use of shaders
// Remove when modifying main.cpp for Assignment 3
//
bool g_useShader = false;
GLuint g_texture = 0;
GLuint g_shader = 0;
GLuint g_waterShader = 0;

// water height
const float WATER_HEIGHT = 0.5f;

// skytexture
//
GLuint g_sky_texture[6] = { 0,0,0,0,0,0 };

bool terrainToggle = true;
bool waterToggle = true;


// Mouse Button callback
// Called for mouse movement event on since the last glfwPollEvents
//
void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
	// cout << "Mouse Movement Callback :: xpos=" << xpos << "ypos=" << ypos << endl;
	if (g_leftMouseDown) {
		g_yaw -= (g_mousePosition.x - xpos) * 0.3;
		g_pitch -= (g_mousePosition.y - ypos) * 0.3;
	}
	g_mousePosition = vec2(xpos, ypos);
}


// Mouse Button callback
// Called for mouse button event on since the last glfwPollEvents
//
void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
	// cout << "Mouse Button Callback :: button=" << button << "action=" << action << "mods=" << mods << endl;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		g_leftMouseDown = (action == GLFW_PRESS);
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (g_useShader) {
			g_useShader = false;
			cout << "Using the default OpenGL pipeline" << endl;
		}
		else {
			g_useShader = true;
			cout << "Using a shader" << endl;
		}
	}
}


// Scroll callback
// Called for scroll event on since the last glfwPollEvents
//
void scrollCallback(GLFWwindow *win, double xoffset, double yoffset) {
	// cout << "Scroll Callback :: xoffset=" << xoffset << "yoffset=" << yoffset << endl;
	g_zoom -= yoffset * g_zoom * 0.2;
	distToCamera -= yoffset * distToCamera * 0.2;
}


// Keyboard callback
// Called for every key event on since the last glfwPollEvents
//
void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	// cout << "Key Callback :: key=" << key << "scancode=" << scancode
	// 	<< "action=" << action << "mods=" << mods << endl;
	// YOUR CODE GOES HERE
    cout << "Key: " << key << ", Scancode: " << scancode << ", Action: " << action << ", Mods: " << mods << endl;
    if (key == GLFW_KEY_M && action == 0) {
        cout << "Toggling wire mode" << endl;
        terrain.toggleWireMode();
    } else if (key == GLFW_KEY_K && action == 0) {
        cout << "Reseeding terrain" << endl;
        int seed = time(NULL);
        cout << "New Seed: " << seed << endl;
        terrain.reseedTerrain(seed);
     }else if(key == GLFW_KEY_T && action == 0) {
     	terrainToggle = !terrainToggle;
     }else if(key == GLFW_KEY_W && action == 0) {
     	waterToggle = !waterToggle;
     }
}


// Character callback
// Called for every character input event on since the last glfwPollEvents
//
void charCallback(GLFWwindow *win, unsigned int c) {
	// cout << "Char Callback :: c=" << char(c) << endl;
	// Not needed for this assignment, but useful to have later on
}


// Sets up where and what the light is
// Called once on start up
// 
void initLight() {
    // Basic Light - GL_LIGHT_0
	float diffintensity[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	float ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    float specular[] = {1.0, 1.0, 1.0, 1.0};

	glLightfv(GL_LIGHT0, GL_POSITION, g_light_pos.dataPointer());
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffintensity);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    
    glEnable(GL_LIGHT0);
}

// An example of how to load a shader from a hardcoded location
//
void initShader() {
	// To create a shader program we use a helper function
	// We pass it an array of the types of shaders we want to compile
	// and the corrosponding locations for the files of each stage
	g_shader = makeShaderProgramFromFile({GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./work/res/shaders/materialShader.vert", "./work/res/shaders/materialShader.frag" });
	g_waterShader = makeShaderProgramFromFile({ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER }, { "./work/res/shaders/waterShader.vert", "./work/res/shaders/waterShader.frag" });

}

// Updates the cameras position
//
void updateCameraPos() {
	float horDist = distToCamera * cos(radians(g_pitch));
	float vertDist = distToCamera * sin(radians(g_pitch));

	float offX = horDist * sin(radians(180 - g_yaw));
	float offZ = horDist * cos(radians(180 - g_yaw));

	g_camera_position.x = 0 - offX;
	g_camera_position.y = vertDist;
	g_camera_position.z = 0 - offZ;
}


// Sets up where the camera is in the scene
// 
void setupCamera(int width, int height) {
	//update the cameras position from the users inputs
	updateCameraPos();

	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fovy, width / float(height), g_znear, g_zfar);

	// Set up the view part of the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(g_camera_position.x, g_camera_position.y, g_camera_position.z,	//position
		g_camera_direction.x, g_camera_direction.y, g_camera_direction.z,		//direction
		g_camera_up.x, g_camera_up.y, g_camera_up.z);	
}

// Draw function
//
void render() {

	// Grey/Blueish background
	glClearColor(0.0, 0.75, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Enable flags for normal rendering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	if(terrainToggle) {
		terrain.renderTerrain(g_shader);
	} 

	// Disable flags for cleanup (optional)
	//glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
}

void renderToBuffer(Watertile wt, GLuint buffer, GLuint texture, double clipPlane[4], bool reflection) {
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_TEXTURE_2D);

	glClearColor(0.0, 0.75, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);

	glPushMatrix();
	if (reflection) {
		glTranslatef(0.0f, 2.0*wt.getWaterPosition().y, 0.0f);
		glScalef(1.0, -1.0, 1.0);
	}
	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, clipPlane);
	render();
	glDisable(GL_CLIP_PLANE0);
	glPopMatrix();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_LESS);
	glDisable(GL_CULL_FACE);
}


vector<Watertile> renderRelfectRefract(vector<Watertile> tiles, int tileWidth) {
	for (int i = 0; i < tileWidth; i++) {
		for (int j = 0; j < tileWidth; j++) {
			tiles[tileWidth*i + j].setCameraPos(g_camera_position);

			//set clip plane for reflection
			double clipPlane[4] = { 0.0, 1.0, 0.0, -tiles[tileWidth*i + j].getWaterPosition().y };

			//render reflection to reflection bufer
			GLuint reflBuf = tiles[tileWidth *i + j].getReflectionBuffer();
			GLuint reflTex = tiles[tileWidth*i + j].getReflectionTexture();
			renderToBuffer(tiles[tileWidth*i + j], reflBuf, reflTex, clipPlane, true);

			//update clip plane for refraction
			clipPlane[1] = -1.0;
			clipPlane[3] = tiles[tileWidth*i + j].getWaterPosition().y;

			//render refraction to refraction buffer
			GLuint refrBuf = tiles[tileWidth*i + j].getRefractionBuffer();
			GLuint refrTex = tiles[tileWidth*i + j].getRefractionTexture();
			renderToBuffer(tiles[tileWidth*i + j], refrBuf, refrTex, clipPlane, false);
		}
	}
	return tiles;
}


// Forward decleration for cleanliness (Ignore)
void APIENTRY debugCallbackARB(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, GLvoid*);


//Main program
// 
int main(int argc, char **argv) {

	// Initialize the GLFW library
	if (!glfwInit()) {
		cerr << "Error: Could not initialize GLFW" << endl;
		abort(); // Unrecoverable error
	}

	// Get the version for GLFW for later
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);

	// Create a windowed mode window and its OpenGL context
	g_window = glfwCreateWindow(640, 480, "Jasen and Matt - Envrionment Simulation", nullptr, nullptr);
	if (!g_window) {
		cerr << "Error: Could not create GLFW window" << endl;
		abort(); // Unrecoverable error
	}

	// Make the g_window's context is current.
	// If we have multiple windows we will need to switch contexts
	glfwMakeContextCurrent(g_window);

	// Initialize GLEW
	// must be done after making a GL context current (glfwMakeContextCurrent in this case)
	glewExperimental = GL_TRUE; // required for full GLEW functionality for OpenGL 3.0+
	GLenum err = glewInit();
	if (GLEW_OK != err) { // Problem: glewInit failed, something is seriously wrong.
		cerr << "Error: " << glewGetErrorString(err) << endl;
		abort(); // Unrecoverable error
	}

	// Print out our OpenGL verisions
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using GLFW " << glfwMajor << "." << glfwMinor << "." << glfwRevision << endl;

	// Attach input callbacks to g_window
	glfwSetCursorPosCallback(g_window, cursorPosCallback);
	glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
	glfwSetScrollCallback(g_window, scrollCallback);
	glfwSetKeyCallback(g_window, keyCallback);
	glfwSetCharCallback(g_window, charCallback);

	// Enable GL_ARB_debug_output if available. Not nessesary, just helpful
	if (glfwExtensionSupported("GL_ARB_debug_output")) {
		// This allows the error location to be determined from a stacktrace
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Set the up callback
		glDebugMessageCallbackARB(debugCallbackARB, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		cout << "GL_ARB_debug_output callback installed" << endl;
	}
	else {
		cout << "GL_ARB_debug_output not available. No worries." << endl;
	}

	initLight();
	initShader();

    terrain.setupTerrain();

    vector<Watertile> tiles;

	int tileWidth = 20; // width of each tile
	int waterWidth = 5; // amount of tiles 
	for (int i = 0; i < waterWidth; i++) {
		for (int j = 0; j < waterWidth; j++) {
			float totalLength = tileWidth*waterWidth;
			float half = totalLength/2;
			float xoff = (half - ((waterWidth - j) * tileWidth)) + tileWidth/2;
			float yoff = (half - ((waterWidth - i) * tileWidth)) + tileWidth/2;
			tiles.push_back(Watertile(vec4(xoff, WATER_HEIGHT,yoff, 0.0f), g_light_pos, g_waterShader, tileWidth));
		}
	}

	//loadSky();

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(g_window)) {

		// Make sure we draw to the WHOLE window
		int width, height;
		glfwGetFramebufferSize(g_window, &width, &height);

		setupCamera(width, height);
		initLight();

		if(waterToggle) {
        	tiles = renderRelfectRefract(tiles, waterWidth);
		}

		// Main Render
		render();
		if(waterToggle) {
	        for (int i = 0; i < pow(waterWidth, 2); i++) {
	            //render water from framebuffers to water quad
	           tiles[i].renderWater();
	        }
    	}
        
		// Swap front and back buffers
		glfwSwapBuffers(g_window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
}






//-------------------------------------------------------------
// Fancy debug stuff
//-------------------------------------------------------------

// function to translate source to string
string getStringForSource(GLenum source) {

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		return("API");
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return("Window System");
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return("Shader Compiler");
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return("Third Party");
	case GL_DEBUG_SOURCE_APPLICATION:
		return("Application");
	case GL_DEBUG_SOURCE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// function to translate severity to string
string getStringForSeverity(GLenum severity) {

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return("HIGH!");
	case GL_DEBUG_SEVERITY_MEDIUM:
		return("Medium");
	case GL_DEBUG_SEVERITY_LOW:
		return("Low");
	default:
		return("n/a");
	}
}

// function to translate type to string
string getStringForType(GLenum type) {
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		return("Error");
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return("Deprecated Behaviour");
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return("Undefined Behaviour");
	case GL_DEBUG_TYPE_PORTABILITY:
		return("Portability Issue");
	case GL_DEBUG_TYPE_PERFORMANCE:
		return("Performance Issue");
	case GL_DEBUG_TYPE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// actually define the function
void APIENTRY debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, GLvoid*) {
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) return;

	cerr << endl; // extra space

	cerr << "Type: " <<
		getStringForType(type) << "; Source: " <<
		getStringForSource(source) << "; ID: " << id << "; Severity: " <<
		getStringForSeverity(severity) << endl;

	cerr << message << endl;

	if (type == GL_DEBUG_TYPE_ERROR_ARB) throw runtime_error("");
}
