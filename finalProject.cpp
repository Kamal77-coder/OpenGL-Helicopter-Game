#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"

#include "heli.550"
#include "bmptotexture.h"

#include <iostream>
#include <string>
#include <vector>
#include "glm/vec3.hpp" // glm::vec3
#include <cmath>

// This is my final project for CS 450 - Intro To Computer Graphics!
//
// There is a helicopter.
// You can fly it.
// 
// Features:
//		Fake skybox
//		Textures
//		Player input
//		Dynamic camera
//
// Author: Kyle Tyler
//
// Boilerplate code was provided in class and can be found here:
//
//	http://web.engr.oregonstate.edu/~mjb/cs550/

// title of these windows
const char *WINDOWTITLE = { "I believe you can fly" };
const char *GLUITITLE = { "User Interface Window" };

// what the glui package defines as true and false:
const int GLUITRUE = { true };
const int GLUIFALSE = { false };

// the escape key:
#define ESCAPE		0x1b

// initial window size:
const int INIT_WINDOW_SIZE = { 600 };

// multiplication factors for input interaction
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:
const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):
const int LEFT = { 4 };
const int MIDDLE = { 2 };
const int RIGHT = { 1 };

// which projection:
enum Projections {
	ORTHO,
	PERSP
};

// which button:
enum ButtonVals {
	RESET,
	QUIT
};

// window background color (rgba):
const GLfloat BACKCOLOR[] = { 0., 0., 0., 1. };

// line width for the axes:
const GLfloat AXES_WIDTH = { 3. };

// the color numbers:
// this order must match the radio button order
enum Colors {
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[] = {
	"Red",
	"Yellow",
	"Green",
	"Cyan",
	"Blue",
	"Magenta",
	"White",
	"Black"
};

// the color definitions:
// this order must match the menu order
const GLfloat Colors[][3] = {
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

// fog parameters:
const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE = { GL_LINEAR };
const GLfloat FOGDENSITY = { 0.30f };
const GLfloat FOGSTART = { 1.5 };
const GLfloat FOGEND = { 4. };

const float skyboxSize = 200.; // size of the play area

// blade parameters:
#define BLADE_RADIUS	1.0
#define BLADE_WIDTH		0.4

// non-constant global variables (boilerplate opengl stuff):
int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to use the z-buffer
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

// display lists
GLuint	HelicopterList;				
GLuint  SkyboxList;
GLuint	BladeList;

// texture IDs
GLuint tex_world_bot; 
GLuint tex_world_top; 
GLuint tex_world_left;
GLuint tex_world_right;
GLuint tex_world_front;
GLuint tex_world_back; 

float	BladeAngle = 0;			// the angle of the helicopter blades
float	Time;					// used for animation (will be value between 0 and 1) 
float	BladeRotationSpeed = .1;
unsigned int MS_IN_THE_ANIMATION_CYCLE = 1000;

// ring starting positions
glm::vec3 ring1(0., 0., 0.);
glm::vec3 ring2(0., -50., 50.);
glm::vec3 ring3(0., 0., 100.);
glm::vec3 ring4(50., 50., 100.);
glm::vec3 ring5(100., 75., 100.);

// camera starting position
glm::vec3 cameraPosition(-1., 9., -25);

// player variables
glm::vec3 playerPosition = glm::vec3(0., 0., 0.); // start at origin
glm::vec3 playerDirection = glm::vec3(0., 0., 1.); // start facing positive z direction
float playerSpeed = .5;
float ascentSpeed = 1.;
float playerAngle;
bool xDecreasing_left = false;
bool zDecreasing_left = true;
bool xDecreasing_right = true;
bool zDecreasing_right = true;

// function prototypes:
void	Animate();
void	Display();
void	DoAxesMenu(int);
void	DoColorMenu(int);
void	DoDepthBufferMenu(int);
void	DoDepthFightingMenu(int);
void	DoDepthMenu(int);
void	DoDebugMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoViewMenu(int);
void	DoRasterString(float, float, float, char *);
void	DoStrokeString(float, float, float, float, char *);
float	ElapsedSeconds();
void	InitGraphics();
void	InitLists();
void	InitMenus();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);
void	Axes(float);
void	HsvRgb(float[3], float[3]);
float	Dot(float v1[3], float v2[3]);
void	Cross(float v1[3], float v2[3], float vout[3]);
float	Unit(float vin[3], float vout[3]);
void	InitTextures();
void	inRing();
unsigned char *BmpToTexture();

// main program:
int main(int argc, char *argv[]) {

	// turn on the glut package
	glutInit(&argc, argv);

	// setup all the graphics stuff:
	InitGraphics();

	// set up textures
	InitTextures();

	// create the display structures that will not change:
	InitLists();

	// init all the global variables used by Display( ):
	// this will also post a redisplay
	Reset();

	// setup all the user interface stuff:
	InitMenus();

	// draw the scene once and wait for some interaction:
	// (this will never return)
	glutSetWindow(MainWindow);
	glutMainLoop();

	// this is here to make the compiler happy:
	return 0;
}

// will be called when main loop has nothing to do 
void Animate() {
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:
	int ms = glutGet(GLUT_ELAPSED_TIME);	// milliseconds
	ms %= MS_IN_THE_ANIMATION_CYCLE;
	Time = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;        // [ 0., 1. )

	MS_IN_THE_ANIMATION_CYCLE++;

	// This isn't the most elegant way of doing it
	// How quickly the Animate()  function executes seems dependent on hardware
	// Even on my own system it would run differently at different times
	// That being said, may need to change scaling for chopper to dropper
	playerPosition.y -= 0.0000001 * MS_IN_THE_ANIMATION_CYCLE;
	
	BladeAngle = Time * 360;
	playerAngle = (atan2(playerDirection.z, playerDirection.x) / 3.14) * 180;

	// force a call to Display( ) next time it is convenient:
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// draw the complete scene:
void Display() {
	if (DebugOn != 0) {
		fprintf(stderr, "Display\n");
	}

	// set which window we want to do the graphics into:
	glutSetWindow(MainWindow);

	// erase the background:
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (DepthBufferOn != 0)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	// specify shading to be flat:
	glShadeModel(GL_FLAT);

	// set the viewport to a square centered in the window:
	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);

	// set the viewing volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 1000.);

	// place the objects into the scene:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set camera parameters
	gluLookAt (
		cameraPosition.x, cameraPosition.y, cameraPosition.z,	// camera position
		playerPosition.x, playerPosition.y, playerPosition.z,	// where camera is looking
		0., 1., 0. // up vector
	); 
	
	// rotate the scene:
	glRotatef((GLfloat)Yrot, 0., 1., 0.);
	glRotatef((GLfloat)Xrot, 1., 0., 0.);

	// uniformly scale the scene:
	if (Scale < MINSCALE)
		Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	
	// set the fog parameters:
	if (DepthCueOn != 0) {
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else {
		glDisable(GL_FOG);
	}

	// possibly draw the axes:
	if (AxesOn != 0) {
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}

	// since we are using glScalef( ), be sure normals get unitized:
	glEnable(GL_NORMALIZE);

	// check if need to change camera angle because we are near a ring
	inRing();

	// draw skybox
	glPushMatrix();
	glTranslatef(playerPosition.x, playerPosition.y, playerPosition.z);
	glRotatef(180., 0., 0., 1.); // flip the skybox cube to correct orientation
	glCallList(SkyboxList);
	glPopMatrix();

	// draw helicopter:

	// Get to right position
	glPushMatrix();
	glTranslatef(playerPosition.x, playerPosition.y, playerPosition.z);
	glRotatef(-playerAngle, 0., 1., 0.);
	glRotatef(90., 0., 1., 0.);
	glTranslatef(0., 0., 0.);
	glRotatef(180., 0., 1., 0.);

	glColor3f(0, 1, 0);
	glCallList(HelicopterList);

	// draw top blade
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(0., 2.9, -2.);		// move to correct position 
	glRotatef(BladeAngle, 0, 1, 0);	// rotate by current spinning rotation angle
	glRotatef(90, 1, 0, 0);			// rotate for proper orientation
	glScalef(5, 5, 5);				// scale to right size
	glCallList(BladeList);			// draw blade
	glPopMatrix();

	// draw back blade
	glPushMatrix();
	glColor3f(0, 0, 1);
	glTranslatef(.5, 2.5, 9.);
	glRotatef(BladeAngle * 3, 1, 0, 0); // needs to rotate 3 times faster than the top blade
	glRotatef(90, 0, 1, 0);
	glScalef(1.5, 1.5, 1.5);
	glCallList(BladeList);
	glPopMatrix();

	glPopMatrix(); // end helicopter

	// draw hoops
	glColor3f(.66, .13, 90.); // light purple

	glPushMatrix();
	glTranslatef(ring1.x, ring1.y, ring1.z);
	glutSolidTorus(1., 10., 10., 10.);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(ring2.x, ring2.y, ring2.z);
	glRotatef(45., 1., 0., 0.);
	glutSolidTorus(1., 10., 10., 10.);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(ring3.x, ring3.y, ring3.z);
	glRotatef(-45., 1., 0., 0.);
	glutSolidTorus(1., 10., 10., 10.);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(ring4.x, ring4.y, ring4.z);
	glRotatef(-45., 1., 0., 0.);
	glRotatef(45., 0., 1., 0.);
	glutSolidTorus(1., 10., 10., 10.);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(ring5.x, ring5.y, ring5.z);
	glRotatef(-45., 1., 0., 0.);
	glRotatef(45., 0., 1., 0.);
	glutSolidTorus(1., 10., 10., 10.);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	// swap the double-buffered framebuffers:
	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	glFlush();
}

// checks if player is near a ring
// if so, change the camera to be near that ring
void inRing() {

	if (playerPosition.x <= ring1.x + 20 && playerPosition.x >= ring1.x - 20 &&
		playerPosition.y <= ring1.y + 20 && playerPosition.y >= ring1.y - 20 &&
		playerPosition.z <= ring1.z + 20 && playerPosition.z >= ring1.z - 20) {
									  								  
		cameraPosition.x = 40.;		  								  
		cameraPosition.y = 10.;		  								  
		cameraPosition.z = 10.;		  								  
	}								  								  
									  								  
	else if (playerPosition.x <= ring2.x + 20 && playerPosition.x >= ring2.x - 20 &&
			 playerPosition.y <= ring2.y + 20 && playerPosition.y >= ring2.y - 20 &&
			 playerPosition.z <= ring2.z + 20 && playerPosition.z >= ring2.z - 20) {

		cameraPosition.x = (ring2.x + ring1.x) * .5 - 30;
		cameraPosition.y = (ring2.y + ring1.y) * .5;
		cameraPosition.z = (ring2.z + ring1.z) * .5;
	}

	else if (playerPosition.x <= ring3.x + 20 && playerPosition.x >= ring3.x - 20 &&
			 playerPosition.y <= ring3.y + 20 && playerPosition.y >= ring3.y - 20 &&
			 playerPosition.z <= ring3.z + 20 && playerPosition.z >= ring3.z - 20) {

		cameraPosition.x = (ring3.x + ring2.x) * .5 - 20;
		cameraPosition.y = (ring3.y + ring2.y) * .5;
		cameraPosition.z = (ring3.z + ring2.z) * .5;
	}

	else if (playerPosition.x <= ring4.x + 20 && playerPosition.x >= ring4.x - 20 &&
			 playerPosition.y <= ring4.y + 20 && playerPosition.y >= ring4.y - 20 &&
			 playerPosition.z <= ring4.z + 20 && playerPosition.z >= ring4.z - 20) {

		cameraPosition.x = (ring4.x + ring3.x) * .5;
		cameraPosition.y = (ring4.y + ring3.y) * .5 + 30;
		cameraPosition.z = (ring4.z + ring3.z) * .5;
	}

	else if (playerPosition.x <= ring5.x + 20 && playerPosition.x >= ring5.x - 20 &&
			playerPosition.y <= ring5.y + 20 && playerPosition.y >= ring5.y - 20 &&
			playerPosition.z <= ring5.z + 20 && playerPosition.z >= ring5.z - 20) {

		cameraPosition.x = (ring5.x + ring4.x) * .5;
		cameraPosition.y = (ring5.y + ring4.y) * .5 + 10;
		cameraPosition.z = (ring5.z + ring4.z) * .5 + 10;
	}
}

void InitTextures() {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	int width = 1024, height = 512;
	int level = 0, ncomps = 3, border = 0;

	glGenTextures(1, &tex_world_back);
	glGenTextures(1, &tex_world_front);
	glGenTextures(1, &tex_world_left);
	glGenTextures(1, &tex_world_right);
	glGenTextures(1, &tex_world_top);
	glGenTextures(1, &tex_world_bot);

	glBindTexture(GL_TEXTURE_2D, tex_world_back);
	unsigned char *Texture1 = BmpToTexture("resources/_red_centre_back.bmp", &width, &height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture1);
	
	glBindTexture(GL_TEXTURE_2D, tex_world_front);
	unsigned char *Texture2 = BmpToTexture("resources/_red_centre_front.bmp", &width, &height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture2);

	glBindTexture(GL_TEXTURE_2D, tex_world_left);
	unsigned char *Texture3 = BmpToTexture("resources/_red_centre_left.bmp", &width, &height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture3);

	glBindTexture(GL_TEXTURE_2D, tex_world_right);
	unsigned char *Texture4 = BmpToTexture("resources/_red_centre_right.bmp", &width, &height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture4);

	glBindTexture(GL_TEXTURE_2D, tex_world_top);
	unsigned char *Texture5 = BmpToTexture("resources/_red_centre_up.bmp", &width, &height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture5);

	glBindTexture(GL_TEXTURE_2D, tex_world_bot);
	unsigned char *Texture6 = BmpToTexture("resources/_red_centre_bottom.bmp", &width, &height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, Texture6);

	glMatrixMode(GL_TEXTURE);
}

void DoAxesMenu(int id) {
	AxesOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoColorMenu(int id) {
	WhichColor = id - RED;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDebugMenu(int id) {
	DebugOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthBufferMenu(int id) {
	DepthBufferOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthFightingMenu(int id) {
	DepthFightingOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthMenu(int id) {
	DepthCueOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// main menu callback:
void DoMainMenu(int id) {
	switch (id) {
	case RESET:
		Reset();
		break;

	case QUIT:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoProjectMenu(int id) {
	WhichProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// use glut to display a string of characters using a raster font:
void DoRasterString(float x, float y, float z, char *s) {
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);

	char c;			// one character to print
	for (; (c = *s) != '\0'; s++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}

// use glut to display a string of characters using a stroke font:
void DoStrokeString(float x, float y, float z, float ht, char *s) {
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05f + 33.33f);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	char c;			// one character to print
	for (; (c = *s) != '\0'; s++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}

// return the number of seconds since the start of the program:
float ElapsedSeconds() {
	// get # of milliseconds since the start of the program:
	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:
	return (float)ms / 1000.f;
}

// initialize the glui window:
void InitMenus() {
	glutSetWindow(MainWindow);

	int numColors = sizeof(Colors) / (3 * sizeof(int));
	int colormenu = glutCreateMenu(DoColorMenu);
	for (int i = 0; i < numColors; i++)	{
		glutAddMenuEntry(ColorNames[i], i);
	}

	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthcuemenu = glutCreateMenu(DoDepthMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int projmenu = glutCreateMenu(DoProjectMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);

	int mainmenu = glutCreateMenu(DoMainMenu);
	glutAddSubMenu("Axes", axesmenu);
	glutAddSubMenu("Colors", colormenu);
	glutAddSubMenu("Depth Buffer", depthbuffermenu);
	glutAddSubMenu("Depth Fighting", depthfightingmenu);
	glutAddSubMenu("Depth Cue", depthcuemenu);
	glutAddSubMenu("Projection", projmenu);
	glutAddMenuEntry("Reset", RESET);
	glutAddSubMenu("Debug", debugmenu);
	glutAddMenuEntry("Quit", QUIT);

	// attach the pop-up menu to the right mouse button:
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
void InitGraphics() {
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

	// open the window and set its title:
	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	// set the framebuffer clear values:
	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(NULL);
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(-1, NULL, 0);

	// set to animate() so it just runs
	glutIdleFunc(Animate);

	// init glew (a window must be open to do this):
#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
}

// initialize the display lists that will not change
void InitLists() {
	glutSetWindow(MainWindow);

	// create the skybox object
	SkyboxList = glGenLists(1);
	glNewList(SkyboxList, GL_COMPILE);

	glEnable(GL_TEXTURE_2D);

	// front
	glBindTexture(GL_TEXTURE_2D, tex_world_front);
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, -1.0);
		glTexCoord2f(0., 0.);	glVertex3f(-skyboxSize, skyboxSize, skyboxSize);
		glTexCoord2f(1., 0.);   glVertex3f(skyboxSize, skyboxSize, skyboxSize);
		glTexCoord2f(1., 1.);	glVertex3f(skyboxSize, -skyboxSize, skyboxSize);
		glTexCoord2f(0., 1.);	glVertex3f(-skyboxSize, -skyboxSize, skyboxSize);
	glEnd();

	// back
	glBindTexture(GL_TEXTURE_2D, tex_world_back);
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(0., 0.);	glVertex3f(skyboxSize, skyboxSize, -skyboxSize);
		glTexCoord2f(1., 0.);   glVertex3f(-skyboxSize, skyboxSize, -skyboxSize);
		glTexCoord2f(1., 1.);	glVertex3f(-skyboxSize, -skyboxSize, -skyboxSize);
		glTexCoord2f(0., 1.);	glVertex3f(skyboxSize, -skyboxSize, -skyboxSize);
	glEnd();

	// left
	glBindTexture(GL_TEXTURE_2D, tex_world_left);
	glBegin(GL_QUADS);
		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(0., 0.);	glVertex3f(-skyboxSize, skyboxSize, -skyboxSize);
		glTexCoord2f(1., 0.);   glVertex3f(-skyboxSize, skyboxSize, skyboxSize);
		glTexCoord2f(1., 1.);	glVertex3f(-skyboxSize, -skyboxSize, skyboxSize);
		glTexCoord2f(0., 1.);	glVertex3f(-skyboxSize, -skyboxSize, -skyboxSize);
	glEnd();

	// right
	glBindTexture(GL_TEXTURE_2D, tex_world_right);
	glBegin(GL_QUADS);
		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(0., 0.);	glVertex3f(skyboxSize, skyboxSize, skyboxSize);
		glTexCoord2f(1., 0.);   glVertex3f(skyboxSize, skyboxSize, -skyboxSize);
		glTexCoord2f(1., 1.);	glVertex3f(skyboxSize, -skyboxSize, -skyboxSize);
		glTexCoord2f(0., 1.);	glVertex3f(skyboxSize, -skyboxSize, skyboxSize);
	glEnd();

	// bottom
	glBindTexture(GL_TEXTURE_2D, tex_world_bot);
	glBegin(GL_QUADS);
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f(0., 0.);	glVertex3f(-skyboxSize, skyboxSize, -skyboxSize);
		glTexCoord2f(1., 0.);   glVertex3f(skyboxSize, skyboxSize, -skyboxSize);
		glTexCoord2f(1., 1.);	glVertex3f(skyboxSize, skyboxSize, skyboxSize);
		glTexCoord2f(0., 1.);	glVertex3f(-skyboxSize, skyboxSize, skyboxSize);
	glEnd();

	// top
	glRotatef(180., 0., 1., 0.);
	glBindTexture(GL_TEXTURE_2D, tex_world_top);
	glBegin(GL_QUADS);
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(0., 0.);	glVertex3f(skyboxSize, -skyboxSize, skyboxSize);
		glTexCoord2f(1., 0.);   glVertex3f(skyboxSize, -skyboxSize, -skyboxSize);
		glTexCoord2f(1., 1.);	glVertex3f(-skyboxSize, -skyboxSize, -skyboxSize);
		glTexCoord2f(0., 1.);	glVertex3f(-skyboxSize, -skyboxSize, skyboxSize);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	// end of skybox
	glEndList();

	// create the helicopter object:
	HelicopterList = glGenLists(1);
	glNewList(HelicopterList, GL_COMPILE);

	// creating helicopter
	int i;
	struct point *p0, *p1, *p2;
	struct tri *tp;
	float p01[3], p02[3], n[3];

	glPushMatrix();
	glTranslatef(0., -1., 0.);
	glRotatef(97., 0., 1., 0.);
	glRotatef(-15., 0., 0., 1.);
	glBegin(GL_TRIANGLES);
	for (i = 0, tp = Helitris; i < Helintris; i++, tp++)
	{
		p0 = &Helipoints[tp->p0];
		p1 = &Helipoints[tp->p1];
		p2 = &Helipoints[tp->p2];

		// fake "lighting" from above:

		p01[0] = p1->x - p0->x;
		p01[1] = p1->y - p0->y;
		p01[2] = p1->z - p0->z;
		p02[0] = p2->x - p0->x;
		p02[1] = p2->y - p0->y;
		p02[2] = p2->z - p0->z;
		Cross(p01, p02, n);
		Unit(n, n);
		n[1] = fabs(n[1]);
		n[1] += .25;
		if (n[1] > 1.)
			n[1] = 1.;
		glColor3f(0., n[1], 0.);

		glVertex3f(p0->x, p0->y, p0->z);
		glVertex3f(p1->x, p1->y, p1->z);
		glVertex3f(p2->x, p2->y, p2->z);
	}
	glEnd();
	glPopMatrix();

	glEndList();

	BladeList = glGenLists(1);
	glNewList(BladeList, GL_COMPILE);

	// draw the helicopter blade with radius BLADE_RADIUS and
	//	width BLADE_WIDTH centered at (0.,0.,0.) in the XY plane
	glBegin(GL_TRIANGLES);
		glVertex2f(BLADE_RADIUS, BLADE_WIDTH / 2.);
		glVertex2f(0., 0.);
		glVertex2f(BLADE_RADIUS, -BLADE_WIDTH / 2.);
		glVertex2f(-BLADE_RADIUS, -BLADE_WIDTH / 2.);
		glVertex2f(0., 0.);
		glVertex2f(-BLADE_RADIUS, BLADE_WIDTH / 2.);
	glEnd();

	glEndList();

	// create the axes:
	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}

// the keyboard callback:
void Keyboard(unsigned char c, int x, int y) {
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c) {
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		DoMainMenu(QUIT);	// will not return here
		break;				// happy compiler
		
	// zoom in
	case 43:
		Scale += 0.1;
		break;

	// zoom out
	case 45:
		Scale -= 0.1;
		break;

	// player controls
	case 'w':
		playerPosition += playerSpeed * playerDirection;
		break;

	case 's':
		playerPosition -= playerSpeed * playerDirection;
		break;

	// This is probably an overcomplicated away of doing things that could be done smarter and faster with math
	// ...but finals are fast approaching and my brain is bad right now
	// Rotating the player direction vector for proper rotation about the y axis
	// this value is to be multiplied by playerSpeed to move in proper direction
	case 'a':
		// between (0, 0, 1) and (1, 0, 0)
		if (!xDecreasing_left && zDecreasing_left) {
			xDecreasing_right = true;
			zDecreasing_right = false;

			playerDirection.x += .05;
			playerDirection.z -= .05;

			if (playerDirection.x >= 1.) {
				xDecreasing_left = true;
				playerDirection.x = 1.;
				playerDirection.z = 0.;
			}
		}
		// between (1, 0, 0) and (0, 0, -1)
		else if (xDecreasing_left && zDecreasing_left) {
			xDecreasing_right = false;
			zDecreasing_right = false;
			
			playerDirection.x -= .05;
			playerDirection.z -= .05;

			if (playerDirection.z <= -1.) {
				zDecreasing_left = false;
				playerDirection.x = 0.;
				playerDirection.z = -1.;
			}
		}
		// between (0, 0, -1) and (-1, 0, 0)
		else if (xDecreasing_left && !zDecreasing_left) {
			xDecreasing_right = false;
			zDecreasing_right = true;

			playerDirection.x -= .05;
			playerDirection.z += .05;

			if (playerDirection.x <= -1.) {
				xDecreasing_left = false;
				playerDirection.x = -1.;
				playerDirection.z = 0.;
			}
		}
		// between (-1, 0, 0) and (0, 0, 1)
		else if (!xDecreasing_left && !zDecreasing_left) {
			xDecreasing_right = true;
			zDecreasing_right = true;

			playerDirection.x += .05;
			playerDirection.z += .05;

			if (playerDirection.z >= 1.) {
				zDecreasing_left = true;
				playerDirection.x = 0.;
				playerDirection.z = 1.;
			}
		}
		break;

	case 'd':
		// between (0, 0, 1) and (-1, 0, 0)
		if (xDecreasing_right && zDecreasing_right) {
			xDecreasing_left = false;
			zDecreasing_left = false;

			playerDirection.x -= .05;
			playerDirection.z -= .05;

			if (playerDirection.x <= -1.) {
				xDecreasing_right = false;
				playerDirection.x = -1.;
				playerDirection.z = 0.;
			}
		}
		// between (-1, 0, 0) and (0, 0, -1)
		else if (!xDecreasing_right && zDecreasing_right) {
			xDecreasing_left = true;
			zDecreasing_left = false;

			playerDirection.x += .05;
			playerDirection.z -= .05;

			if (playerDirection.z <= -1.) {
				zDecreasing_right = false;
				playerDirection.x = 0.;
				playerDirection.z = -1.;
			}
		}
		// between (0, 0, -1) and (1, 0, 0)
		else if (!xDecreasing_right && !zDecreasing_right) {
			xDecreasing_left = true;
			zDecreasing_left = true;

			playerDirection.x += .05;
			playerDirection.z += .05;

			if (playerDirection.x >= 1.) {
				xDecreasing_right = true;
				playerDirection.x = 1.;
				playerDirection.z = 0.;
			}
		}
		// between (1, 0, 0) and (0, 0, 1)
		else if (xDecreasing_right && !zDecreasing_right) {
			xDecreasing_left = false;
			zDecreasing_left = true;

			playerDirection.x -= .05f;
			playerDirection.z += .05f;

			if (playerDirection.z >= 1.) {
				zDecreasing_right = true;
				playerDirection.x = 0.;
				playerDirection.z = 1.;
			}
		}
		break;

	case ' ':
		// don't let the animation cycle get below 100 ms
		if(MS_IN_THE_ANIMATION_CYCLE >= 300)
			MS_IN_THE_ANIMATION_CYCLE -= 200.;

		// make sure this number doesn't get too out of control
		if(MS_IN_THE_ANIMATION_CYCLE >= 10000)
			MS_IN_THE_ANIMATION_CYCLE = 10000.;

		playerPosition.y += ascentSpeed;

		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// called when the mouse button transitions down or up:
void MouseButton(int button, int state, int x, int y) {
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);

	// get the proper button bit mask:
	switch (button) {
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:
	if (state == GLUT_DOWN)	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else {
		ActiveButton &= ~b;		// clear the proper bit
	}
}

// called when the mouse moves while a button is down:
void MouseMotion(int x, int y) {
	if (DebugOn != 0)
		fprintf(stderr, "MouseMotion: %d, %d\n", x, y);

	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0) {
		Xrot += (ANGFACT*dy);
		Yrot += (ANGFACT*dx);
	}

	if ((ActiveButton & MIDDLE) != 0) {
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
void Reset() {
	ActiveButton = 0;
	AxesOn = 0;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale = 1.0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
}

// called when user resizes the window:
void Resize(int width, int height) {
	if (DebugOn != 0)
		fprintf(stderr, "ReSize: %d, %d\n", width, height);

	// don't really need to do anything since window size is
	// checked each time in Display( ):
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// handle a change to the window's visibility:
void Visibility(int state) {
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE) {
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
}

///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrt(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}

// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
	0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
	-.5f, .5f, .5f, -.5f
};

static int xorder[] = {
	1, 2, -3, 4
};

static float yx[] = {
	0.f, 0.f, -.5f, .5f
};

static float yy[] = {
	0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
	1, 2, 3, -2, 4
};

static float zx[] = {
	1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
	.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
	1, 2, 3, 4, -5, 6
};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
void Axes(float length) {
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++) {
		int j = xorder[i];
		if (j < 0) {
			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact*xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++) {
		int j = yorder[i];
		if (j < 0) {
			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact*yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++) {
		int j = zorder[i];
		if (j < 0) {
			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact*zy[j], base + fact * zx[j]);
	}
	glEnd();
}

// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );
void HsvRgb(float hsv[3], float rgb[3]) {
	// guarantee valid input:
	float h = hsv[0] / 60.f;
	while (h >= 6.)	h -= 6.;
	while (h <  0.) 	h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:
	if (s == 0.0) {
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	float i = floor(h);
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - s * f);
	float t = v * (1.f - (s * (1.f - f)));

	float r, g, b;			// red, green, blue
	switch ((int)i) {
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}

	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}