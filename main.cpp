/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: A4
 *  File: main.cpp
 *
 *  Author: Michael Niemeyer
 *	
 *  Description:
 *	Draw a moving mascot around the vehicle. 
 *  The mascot moves along a bezier curve
 *  
 */

// include the OpenGL library header
#ifdef __APPLE__ // if compiling on Mac OS
#include <OpenGL/gl.h>
#else // if compiling on Linux or Windows OS
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h> // include GLFW framework header

#include <CSCI441/objects.hpp> // for our 3D objects

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>   // for cos(), sin() functionality
#include <stdio.h>  // for printf functionality
#include <stdlib.h> // for exit functionality
#include <time.h>   // for time() functionality

#include <fstream> // for file I/O
#include <vector>  // for vector
#include <cmath>   // for math

#include "mniemeyer.cpp"
#include "kduong.cpp"
#include "mvillafuerte.cpp"
using namespace std;
//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 800, windowHeight = 600;

int leftMouseButton, control; // status of the mouse button and control
glm::vec2 mousePos;			  // last known X and Y of the mouse

glm::vec3 camPos;					   // camera position in cartesian coordinates
float cameraTheta, cameraPhi;		   // camera DIRECTION in spherical coordinates
glm::vec3 camDir = glm::vec3(0, 0, 0); // camera DIRECTION in cartesian coordinates
float camZoom;

GLuint environmentDL; // display list for the 'city'

// Global for Coaster in RCT
vector<glm::vec3> coasterPoints;
vector<glm::vec3> coasterPath;
int iteratorKD = 0;
float prevDir = 0;

float patchRes = 0.05;

// Global for Cameras in RCT
int cameraNumber = 1;
int fpvCamera = 1;
int view = 1;
glm::vec3 fpvPos;
bool FPV = false;

//*************************************************************************************
//
// Helper Functions

// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////
float getRand() { return rand() / (float)RAND_MAX; }

bool loadControlPointsKD2(char *filename)
{
	// TODO #02: read in control points from file.  Make sure the file can be
	// opened and handle it appropriately.

	// Sanity check
	fprintf(stdout, "LOADING ");
	fprintf(stdout, filename);
	fprintf(stdout, "...\n\n\n");

	// Checking if it is .csv
	string extension = "@@@@";
	for (int i = 0; i < 4; i++)
	{
		extension[i] = filename[strlen(filename) - 4 + i];
	}

	if (extension != ".csv")
	{
		fprintf(stderr, "[ERROR]: INVALID FILE TYPE. ENSURE IT IS A .CSV\n");
		exit(EXIT_FAILURE);
	}

	ifstream input(filename);
	string line;

	// Checking if file exists and was sucessfully opened
	if (!input.is_open())
	{
		fprintf(stderr, "[ERROR]: UNABLE TO OPEN FILE\n");
		exit(EXIT_FAILURE);
	}

	// Loading Bezier Patch
	int k = 0;
	input >> k;
	for (int i = 0; i < k; i++)
	{
		string a, b, c;
		getline(input, a, ',');
		getline(input, b, ',');
		getline(input, c);

		stringstream aa(a);
		stringstream bb(b);
		stringstream cc(c);

		int x, y, z = 0;
		aa >> x;
		bb >> y;
		cc >> z;

		p.push_back(glm::vec3(x, y, z));
	}

	// Loading Coaster Track
	int n = 0;
	input >> n;
	for (int i = 0; i < n; i++)
	{
		string a, b, c;
		getline(input, a, ',');
		getline(input, b, ',');
		getline(input, c);

		stringstream aa(a);
		stringstream bb(b);
		stringstream cc(c);

		int x, y, z = 0;
		aa >> x;
		bb >> y;
		cc >> z;

		coasterPoints.push_back(glm::vec3(x, y, z));
	}
	input.close();
	populatePath(coasterPoints, coasterPath);

	return true;
}

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta or cameraPhi is updated.
//
////////////////////////////////////////////////////////////////////////////////

void recomputeOrientation()
{
	if (cameraNumber == 4)
	{
		camDir = glm::vec3(sin(cameraTheta) * sin(cameraPhi), -cos(cameraPhi), -cos(cameraTheta) * sin(cameraPhi));
		camDir = glm::normalize(camDir);
	}
	else
	{
		camDir = glm::vec3(camZoom * sin(cameraTheta) * sin(cameraPhi), camZoom * -cos(cameraPhi), camZoom * -cos(cameraTheta) * sin(cameraPhi));
	}
}

//*************************************************************************************
//
// Event Callbacks

//
//	void error_callback( int error, const char* description )
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
static void error_callback(int error, const char *description)
{
	fprintf(stderr, "[ERROR]: %s\n", description);
}

//Read the input from the keyboard
static void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	//Start actions when a key is pressed
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
		case GLFW_KEY_Q:
			exit(EXIT_SUCCESS);
		case GLFW_KEY_W:
			vMove += 1;
			break;
		case GLFW_KEY_S:
			vMove -= 1;
			break;
		case GLFW_KEY_A:
			vRot += 1;
			break;
		case GLFW_KEY_D:
			vRot -= 1;
			break;
		case GLFW_KEY_C:
			showControl = !showControl;
			hideControlPoint = !hideControlPoint;
			hideControlCage = !hideControlCage;
			hideControlPath = !hideControlPath;
			point = !point;
			connection = !connection;
			curve = !curve;
			break;
		case GLFW_KEY_F:
			FPV = !FPV;
			break;
		case GLFW_KEY_1:
			cameraNumber = 1;
			recomputeOrientation();
			break;
		case GLFW_KEY_2:
			cameraNumber = 2;
			recomputeOrientation();
			break;
		case GLFW_KEY_3:
			cameraNumber = 3;
			recomputeOrientation();
			break;
		case GLFW_KEY_4:
			cameraNumber = 4;
			recomputeOrientation();
			break;
		case GLFW_KEY_J:
			fpvCamera = 1;
			recomputeOrientation();
			break;
		case GLFW_KEY_K:
			fpvCamera = 2;
			recomputeOrientation();
			break;
		case GLFW_KEY_L:
			fpvCamera = 3;
			recomputeOrientation();
		}
	}

	if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
	{
		control = action;
	}
	//Stop actions when the key is released
	if (action == GLFW_RELEASE)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			vMove -= 1;
			break;
		case GLFW_KEY_S:
			vMove += 1;
			break;
		case GLFW_KEY_A:
			vRot -= 1;
			break;
		case GLFW_KEY_D:
			vRot += 1;
			break;
		}
	}
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
		{
		case GLFW_KEY_UP:
			camPos += camDir * 5.0f;
			break;
		case GLFW_KEY_DOWN:
			camPos -= camDir * 5.0f;
		}
	}
}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//  GLFW callback for mouse movement. We update cameraPhi and/or cameraTheta
//      based on how much the user has moved the mouse in the
//      X or Y directions (in screen space) and whether they have held down
//      the left or right mouse buttons. If the user hasn't held down any
//      buttons, the function just updates the last seen mouse X and Y coords.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback(GLFWwindow *window, double x, double y)
{
	if (leftMouseButton == GLFW_PRESS || leftMouseButton == GLFW_REPEAT)
	{
		if (control == GLFW_PRESS || control == GLFW_REPEAT)
		{
			camZoom += .25 * (mousePos.y - y);
			if (camZoom < 5)
				camZoom = 5;
		}
		else
		{
			cameraTheta += 0.005 * (x - mousePos.x);
			cameraPhi += 0.005 * (mousePos.y - y);
			if (cameraPhi > M_PI)
				cameraPhi = M_PI - 0.0001;
			else if (cameraPhi < M_PI / 2 && cameraNumber != 4)
				cameraPhi = M_PI / 2;
			else if (cameraPhi < 0 && cameraNumber == 4)
				cameraPhi = 0.0001;
		}
		recomputeOrientation(); // update camera (x,y,z) based on (theta,phi)
	}

	mousePos.x = x;
	mousePos.y = y;
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//  GLFW callback for mouse clicks. We save the state of the mouse button
//      when this is called so that we can check the status of the mouse
//      buttons inside the motion callback (whether they are up or down).
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		leftMouseButton = action;
	}
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// drawCity() //////////////////////////////////////////////////////////////////
//
//  Function to draw a random city using CSCI441 3D Cubes
//
////////////////////////////////////////////////////////////////////////////////
void drawCity()
{
	for (int i = -50; i < 50; i++)
	{
		for (int j = -50; j < 50; j++)
		{
			if (i % 2 == 0 && j % 2 == 0 && getRand() < 0.1)
			{
				int r = rand() % 10 + 1;
				glm::mat4 loc = glm::translate(glm::mat4(), glm::vec3(i, (double)r / 2.0, j));
				glMultMatrixf(&loc[0][0]);
				glm::mat4 scl = glm::scale(glm::mat4(), glm::vec3(1, r, 1));
				glMultMatrixf(&scl[0][0]);
				glColor3f(getRand(), getRand(), getRand());
				CSCI441::drawSolidCube(1);
				glMultMatrixf(&(glm::inverse(scl))[0][0]);
				glMultMatrixf(&(glm::inverse(loc))[0][0]);
			}
		}
	}
}

//Render the bezier patch
void renderBezierPatch()
{
	glm::vec3 point, normal;
	glColor3f(0, .5, 0);
	for (float u = 0; u <= 1; u += patchRes)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (float v = 0; v <= 1 + patchRes; v += patchRes)
		{
			point = evaluateBezierPatch(u, v);
			normal = normalPatch(u, v);
			glNormal3f(normal.x, normal.y, normal.z);
			glVertex3f(point.x, point.y, point.z);

			point = evaluateBezierPatch(u + patchRes, v);
			normal = normalPatch(u + patchRes, v);
			glNormal3f(normal.x, normal.y, normal.z);
			glVertex3f(point.x, point.y, point.z);
		}
		glEnd();
	}
}

// generateEnvironmentDL() /////////////////////////////////////////////////////
//
//  This function creates a display list with the code to draw a simple
//      environment for the user to navigate through.
//
//  And yes, it uses a global variable for the display list.
//  I know, I know! Kids: don't try this at home. There's something to be said
//      for object-oriented programming after all.
//
////////////////////////////////////////////////////////////////////////////////
void generateEnvironmentDL()
{
	// TODO #1 Create a Display List & Call our Drawing Functions
	environmentDL = glGenLists(1);
	glNewList(environmentDL, GL_COMPILE);
	//drawCity();
	//drawGrid();
	glEndList();
}

void drawCoasterTrack()
{
	glDisable(GL_LIGHTING);
	glLineWidth(5.0f);
	glColor3f(1, 1, 0);
	for (unsigned int i = 0; i < coasterPoints.size() - 3; i += 3)
	{
		glm::vec3 v1 = coasterPoints.at(i);
		glm::vec3 v2 = coasterPoints.at(i + 1);
		glm::vec3 v3 = coasterPoints.at(i + 2);
		glm::vec3 v4 = coasterPoints.at(i + 3);
		renderBezierCurve(v1, v2, v3, v4, 10);
	}
	glLineWidth(1.0f);
	glEnable(GL_LIGHTING);

	for (unsigned int i = 0; i < arcPath.size(); i += 1000)
	{

		glm::mat4 transMtx = glm::translate(glm::mat4(), arcPath.at(i));
		glMultMatrixf(&transMtx[0][0]);
		{
			glm::mat4 scaleMtx = glm::scale(glm::mat4(), glm::vec3(10, 1, 10));
			glMultMatrixf(&scaleMtx[0][0]);
			{
				//glColor3f(216.0/255.0,101.0/255.0,101.0/255.0);
				glColor3f(1, 1, 0);
				CSCI441::drawSolidCube(.1);
			};
			glMultMatrixf(&(glm::inverse(scaleMtx))[0][0]);
		};
		glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
	}
}

void updateKhanh()
{
	iter = (iter + 1) % mascotPath.size();
	wheelAng = fmod((wheelAng + 10.0f), 360.0f);
	mascotAngle = fmod((mascotAngle + .02f), 360.0f);

	iteratorKD = (iteratorKD + 1) % coasterPath.size();
	carPos = coasterPath.at(iteratorKD); // Updating Khanh's car, so camera can follow and to travel the track

	// Updating Khanh's car direction
	glm::vec3 v1 = coasterPath.at((iteratorKD + 1) % coasterPath.size());
	glm::vec3 v2 = coasterPath.at(iteratorKD);
	glm::vec3 v3 = v2 - v1;
	// Restricting dot product / magnitude to 2D, y shouldn't affect rotation
	// Also, take into account acos goes only from 0 to 180
	float newDir = acos((v3.x * 0 + v3.z * -1) / sqrt(v3.x * v3.x + v3.z * v3.z)) * 180.0f / 3.14f;
	if (newDir > prevDir)
	{
		carDir = 360.0f - newDir;
	}
	else
	{
		carDir = newDir;
	}
	prevDir = newDir;
}

void updateMike()
{
	++path;
	++arcPos;

	if (arcPos + 1 > (int)arcPath.size())
	{
		arcPos = 0;
	}

	if (path > (int)controlPath.size())
	{
		path = 0;
	}

	arcPoint = arcPath.at(arcPos);
	carPosM = arcPoint;
	controlPoint = controlPath.at(path);

	glm::vec3 v1 = arcPoint.at((arcPos + 1);
	glm::vec3 v2 = coasterPath.at(arcPos);
	glm::vec3 v3 = v2 - v1;
	float newDir = acos((v3.x * 0 + v3.z * -1) / sqrt(v3.x * v3.x + v3.z * v3.z)) * 180.0f / 3.14f;
	if (newDir > prevDir)
	{
		carTheta = 360.0f - newDir;
	}
	else
	{
		carTheta = newDir;
	}
	prevDir = newDir;
	//cout << "Main path: " << path << endl;
}

//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene(void)
{
	glCallList(environmentDL);
	renderBezierPatch();
	drawCoasterTrack();

	drawMN();

	drawKD();

	if (fpvCamera == 3 && view == 2)
	{
		// Do Nothing
	}
	else
	{
		drawMV();
	}
}

//*************************************************************************************
//
// Setup Functions

//
//  void setupGLFW()
//
//      Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
GLFWwindow *setupGLFW()
{
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback(error_callback);

	// initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "[ERROR]: Could not initialize GLFW\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf(stdout, "[INFO]: GLFW initialized\n");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // request OpenGL v2.X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // request OpenGL v2.1
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);	// do not allow our window to be able to be resized

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Mascot", NULL, NULL);
	if (!window)
	{ // if the window could not be created, NULL is returned
		fprintf(stderr, "[ERROR]: GLFW Window could not be created\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf(stdout, "[INFO]: GLFW Window created\n");
	}

	glfwMakeContextCurrent(window); // make the created window the current window
	glfwSwapInterval(1);			// update our screen after at least 1 screen refresh

	glfwSetKeyCallback(window, keyboard_callback);			   // set our keyboard callback function
	glfwSetCursorPosCallback(window, cursor_callback);		   // set our cursor position callback function
	glfwSetMouseButtonCallback(window, mouse_button_callback); // set our mouse button callback function

	return window; // return the window that was created
}

//
//  void setupOpenGL()
//
//      Used to setup everything OpenGL related.  For now, the only setting
//	we need is what color to make the background of our window when we clear
//	the window.  In the future we will be adding many more settings to this
//	function.
//
void setupOpenGL()
{
	// tell OpenGL to perform depth testing with the Z-Buffer to perform hidden
	//		surface removal.  We will discuss this more very soon.
	glEnable(GL_DEPTH_TEST);

	//******************************************************************
	// this is some code to enable a default light for the scene;
	// feel free to play around with this, but we won't talk about
	// lighting in OpenGL for another couple of weeks yet.
	float lightCol[4] = {1, 1, 1, 1};
	float ambientCol[4] = {0.0, 0.0, 0.0, 1.0};
	float lPosition[4] = {10, 10, 10, 1};
	glLightfv(GL_LIGHT0, GL_POSITION, lPosition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientCol);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// tell OpenGL not to use the material system; just use whatever we
	// pass with glColor*()
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	//******************************************************************

	// tells OpenGL to blend colors across triangles. Once lighting is
	// enabled, this means that objects will appear smoother - if your object
	// is rounded or has a smooth surface, this is probably a good idea;
	// if your object has a blocky surface, you probably want to disable this.
	glShadeModel(GL_SMOOTH);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set the clear color to black
}

//
//  void setupScene()
//
//      Used to setup everything scene related.  Give our camera an
//	initial starting point and generate the display list for our city
//
void setupScene()
{
	// give the camera a scenic starting point.
	camZoom = 10.0f;
	cameraTheta = 2.0f * M_PI / 3.0f;
	cameraPhi = M_PI / 1.5f;
	recomputeOrientation();
	MNSetUp();
	srand(time(NULL)); // seed our random number generator
	generateEnvironmentDL();
}

///*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main(int argc, char *argv[])
{

	// Read control points from CSV file

	loadControlPointsMN("controlPoints.csv");
	loadControlPointsKD("controlPointsSpiral.csv");
	loadControlPointsMV("controlPoints13.csv");

	//loadControlPointsKD("CoasterPoints.csv", coasterPoints, coasterPath);
	cout << "its the functions" << endl;
	loadControlPointsKD2("RCT.csv");
	readControlPointsMV(coasterPoints);

	// GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW(); // initialize all of the GLFW specific information releated to OpenGL and our window
	setupOpenGL();					  // initialize all of the OpenGL specific information
	setupScene();					  // initialize objects in our scene

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while (!glfwWindowShouldClose(window))
	{														// check if the window was instructed to be closed
		glDrawBuffer(GL_BACK);								// work with our back frame buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the current color contents and depth buffer in the window

		moveMascot();
		rotateCurve();
		moveVehicle();

		//Change the angle of the vehicle
		if (vRot != 0)
		{
			vTheta += vRot * 0.05f;
			recomputeVehicleDirection();
		}

		// Update global constants for animation
		updateKhanh();
		updateMike();

		// update the projection matrix based on the window size
		// the GL_PROJECTION matrix governs properties of the view coordinates;
		// i.e. what gets seen - use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projMtx = glm::perspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.001f, 1000.0f);
		glMatrixMode(GL_PROJECTION);   // change to the Projection matrix
		glLoadIdentity();			   // set the matrix to be the identity
		glMultMatrixf(&projMtx[0][0]); // load our orthographic projection matrix into OpenGL's projection matrix state

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		GLint framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport(0, 0, framebufferWidth, framebufferHeight);

		glMatrixMode(GL_MODELVIEW); // make the ModelView matrix current to be modified by any transformations
		glLoadIdentity();			// set the matrix to be the identity

		// set up our look at matrix to position our camera
		glm::mat4 viewMtx;
		switch (cameraNumber)
		{
		case 1:
			camPos = vLocXYZ + camDir;
			viewMtx = glm::lookAt(camPos, vLocXYZ, glm::vec3(0, 1, 0));
			break;
		case 2:
			camPos = carPos + camDir;
			viewMtx = glm::lookAt(camPos, carPos, glm::vec3(0, 1, 0));
			break;
		case 3:
			camPos = carPosM + camDir;
			viewMtx = glm::lookAt(camPos, carPosM, glm::vec3(0, 1, 0));
			break;
		case 4:
			viewMtx = glm::lookAt(camPos, camPos + camDir, glm::vec3(0, 1, 0));
		}
		// multiply by the look at matrix - this is the same as our view martix
		glMultMatrixf(&viewMtx[0][0]);

		view = 1;
		renderScene(); // draw everything to the window
		if (FPV)
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(0, 0, 200, 200);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glViewport(0, 0, 200, 200); // to draw minimap
			glm::mat4 fpvMtx;
			glm::vec3 t;
			switch (fpvCamera)
			{
			case 1:
				fpvMtx = glm::lookAt(vLocXYZ + glm::vec3(0, .8, 0), vLocXYZ + vDir + glm::vec3(0, .8, 0), glm::vec3(0, 1, 0));
				break;
			case 2:
				t = glm::vec3(glm::sin(carDir * 3.14f / 180.0f), 0, glm::cos(carDir * 3.14f / 180.0f));
				fpvMtx = glm::lookAt(carPos + glm::vec3(0, 1.95, 0), carPos + t + glm::vec3(0, 1.95, 0), glm::vec3(0, 1, 0));
				break;
			case 3:
				t = glm::vec3(glm::sin(carTheta * 3.14f / 180.0f), 0, glm::cos(carTheta * 3.14f / 180.0f));
				fpvMtx = glm::lookAt(carPosM + glm::vec3(0, 1, 0), carPosM + t + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
				break;
			}
			glMultMatrixf(&fpvMtx[0][0]);

			view = 2;
			renderScene();
			glDisable(GL_SCISSOR_TEST);
		}

		glfwSwapBuffers(window); // flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();		 // check for any events and signal to redraw screen
	}

	glfwDestroyWindow(window); // clean up and close our window
	glfwTerminate();		   // shut down GLFW to clean up our context

	return EXIT_SUCCESS; // exit our program successfully!
}
