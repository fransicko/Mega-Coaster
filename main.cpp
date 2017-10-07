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
#ifdef __APPLE__					// if compiling on Mac OS
 	#include <OpenGL/gl.h>
#else										// if compiling on Linux or Windows OS
 	#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>	// include GLFW framework header

#include <CSCI441/objects.hpp> // for our 3D objects

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>				// for cos(), sin() functionality
#include <stdio.h>			// for printf functionality
#include <stdlib.h>			// for exit functionality
#include <time.h>			  // for time() functionality


#include <fstream>			// for file I/O
#include <vector>				// for vector
using namespace std;
//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 640, windowHeight = 480;

int leftMouseButton, control;    	 									// status of the mouse button and control
glm::vec2 mousePos;			              		  // last known X and Y of the mouse

glm::vec3 camPos;            							 	// camera position in cartesian coordinates
float cameraTheta, cameraPhi;               // camera DIRECTION in spherical coordinates
glm::vec3 camDir; 			                    // camera DIRECTION in cartesian coordinates
float camZoom;

int vMove = 0;
int vRot = 0;
float aRot = 0.0f;
glm::vec3 vLoc;
glm::vec3 vDir;
float vTheta;
glm::vec3 mascotLoc;
int currentPoint = 0;
float extend = 0.0f;
float extDir = 1.0f;

GLuint environmentDL;                       // display list for the 'city'

vector<glm::vec3> controlPoints;
float trackPointVal = 0.0f;
int numSegments = 0;
glm::vec3 center(0);

vector<glm::vec3> points;
float curveRot = 0.0f;

bool showControl = true;


int numLines;

//*************************************************************************************
//
// Helper Functions

// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////
float getRand() { return rand() / (float)RAND_MAX; }

//Calculate each point to draw on the curve
void evaluateBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int resolution ) {
	float t = 0.0f;
	for(int i = 0; i < resolution; i++){
		glm::vec3 point(0,0,0);
		glm::vec3 a, b, c;
		a = -p0 + 3.0f*p1 - 3.0f*p2 + p3;
		b = 3.0f*p0 - 6.0f*p1 + 3.0f*p2;
		c = -3.0f*p0 + 3.0f*p1;	
		point = a*glm::pow(t, 3.0f) + b*glm::pow(t, 2.0f) + c*t + p0;
		points.push_back(point);
		t += (float)1/resolution;
	}
}

// loadControlPoints() /////////////////////////////////////////////////////////
//
//  Load our control points from file and store them in
//	the global variable controlPoints
//
////////////////////////////////////////////////////////////////////////////////
bool loadControlPoints( char* filename ) {
	FILE *file = fopen(filename, "r");
	if(file!=NULL){
		int numPoints;
			fscanf(file, "%i", &numPoints);
			numLines = (numPoints - 1)/3;
			for(int i = 0; i < numPoints; i++){
				glm::vec3 v;
				if(fscanf(file, "%f, %f, %f", &v.x, &v.y, &v.z) == 3){
					center += v;
					controlPoints.push_back(v);
				}else return false;
			}
		center = center /(float) numPoints;
		for(int i = 0; i < (int)controlPoints.size()-1; i += 3){
			glm::vec3 p0 = controlPoints.at(i);
			glm::vec3 p1 = controlPoints.at(i+1);
			glm::vec3 p2 = controlPoints.at(i+2);
			glm::vec3 p3 = controlPoints.at(i+3);
			float totalDistance = glm::distance(p0, p1) + glm::distance(p1, p2) + glm::distance(p2, p3);
			evaluateBezierCurve(p0, p1, p2, p3, (int) 5000/totalDistance);
		}
	}else return false;
	return true;
}




// renderBezierCurve() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////
void renderBezierCurve() {
	glDisable(GL_LIGHTING);
	glLineWidth(3.0f);
	glColor3f(0, 0, 1);
	glBegin(GL_LINE_STRIP);
	for(int i = 0; i < (int)points.size(); i++){
		glVertex3f(points[i].x, points[i].y, points[i].z);
	}
	glEnd();
	
	glEnable(GL_LIGHTING);
}

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta or cameraPhi is updated.
//
////////////////////////////////////////////////////////////////////////////////

void recomputeOrientation() {

	camPos = glm::vec3(camZoom*glm::sin(cameraTheta)*glm::sin(cameraPhi),
			-camZoom*glm::cos(cameraPhi), 
			-camZoom*glm::cos(cameraTheta)*glm::sin(cameraPhi));
}

//Change the direction of the vehicle when the A or D key is pressed
void recomputeVehicleDirection(){
	
	vDir = glm::vec3(-glm::sin(vTheta), 0, -glm::cos(vTheta));
	
	//Normalize the direction vector
	vDir = glm::normalize(vDir);
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
static void error_callback( int error, const char* description ) {
	fprintf( stderr, "[ERROR]: %s\n", description );
}

//Read the input from the keyboard
static void keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
	//Start actions when a key is pressed
	if( action == GLFW_PRESS ) {
		switch( key ) {
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
				vRot +=1;
				break;
			case GLFW_KEY_D:
				vRot -=1;
				break;
			case GLFW_KEY_C:
				showControl = !showControl;
				break;
		}
	}
	
	if(key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL){
		control = action;
	}
	//Stop actions when the key is released
	if(action == GLFW_RELEASE) {
		switch(key){
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
static void cursor_callback( GLFWwindow *window, double x, double y ) {
	if( leftMouseButton == GLFW_PRESS || leftMouseButton == GLFW_REPEAT) {
		if(control == GLFW_PRESS || control == GLFW_REPEAT){
			camZoom += (y-mousePos.y)*0.01;
		}else {
			cameraTheta -= (x-mousePos.x)*0.005;
			float newPhi = cameraPhi - (y-mousePos.y)*0.005;
			if(newPhi > 0 && newPhi < M_PI){
				cameraPhi = newPhi ;
			}
		}
		recomputeOrientation();     // update camera (x,y,z) based on (theta,phi)
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
static void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
	if( button == GLFW_MOUSE_BUTTON_LEFT ) {
		leftMouseButton = action;
	}
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// drawGrid() //////////////////////////////////////////////////////////////////
//
//  Function to draw a grid in the XZ-Plane using OpenGL 2D Primitives (GL_LINES)
//
////////////////////////////////////////////////////////////////////////////////
void drawGrid() {
    /*
     *	We will get to why we need to do this when we talk about lighting,
     *	but for now whenever we want to draw something with an OpenGL
     *	Primitive - like a line, quad, point - we need to disable lighting
     *	and then reenable it for use with the CSCI441 3D Objects.
     */
    glDisable( GL_LIGHTING );

    /** TODO #3: DRAW A GRID IN THE XZ-PLANE USING GL_LINES **/
	for (int i = -50; i < 50; i++) {
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINES);
		glVertex3f(i, 0, 50);
		glVertex3f(i, 0, -50);
		glVertex3f(50, 0, i);
		glVertex3f(-50, 0, i);
		glEnd();
	}

    /*
     *	As noted above, we are done drawing with OpenGL Primitives, so we
     *	must turn lighting back on.
     */
    glEnable( GL_LIGHTING );
}

//Draw the lines conecting the control points
void drawControl(){
	glDisable(GL_LIGHTING);
		glColor3f(1, 1, 0);
		glLineWidth(3.0f);
		glBegin(GL_LINE_STRIP);
		for(int i = 0; i < (int) controlPoints.size(); i++){
			glVertex3f(controlPoints.at(i).x, controlPoints.at(i).y, controlPoints.at(i).z);
		}
		glEnd();
		glEnable(GL_LIGHTING);
}

//Draw the points read in from the file
void drawPoint(glm::vec3 v){
	glColor3f(0, 1, 0);
	glm::mat4 trans = glm::translate(glm::mat4(), v);
	glMultMatrixf(&trans[0][0]);
	CSCI441::drawSolidSphere(0.3, 16, 16);
	glMultMatrixf(&(glm::inverse(trans))[0][0]);
}

// drawCity() //////////////////////////////////////////////////////////////////
//
//  Function to draw a random city using CSCI441 3D Cubes
//
////////////////////////////////////////////////////////////////////////////////
void drawCity() {
	for (int i = -50; i < 50; i++) {
		for (int j = -50; j < 50; j++) {
			if (i % 2 == 0 && j % 2 == 0 && getRand() < 0.1) {
				int r = rand() % 10 + 1;
				glm::mat4 loc = glm::translate(glm::mat4(), glm::vec3(i,(double) r/2.0, j));
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
//Draw the rectangle body of the vehicle
void drawBody(){
	glColor3f(0.545, 0.271, 0.075);
	glm::mat4 scl = glm::scale(glm::mat4(), glm::vec3(1.0, 0.1, 1.5));
		glMultMatrixf(&scl[0][0]);
		CSCI441::drawSolidCube(1);
		glMultMatrixf(&(glm::inverse(scl))[0][0]);
}

//Draw a wheel of the vehicle
void drawWheel(){
	glColor3f(1.000, 0.714, 0.757);
	glm::mat4 transWheel = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
	glMultMatrixf(&transWheel[0][0]);
	CSCI441::drawSolidTorus(0.12, 0.2, 8, 8);
	glMultMatrixf(&(glm::inverse(transWheel))[0][0]);
}

//Draw the axle and wheel of the vehicle and rotate when the vehicle is moving
void drawAxle(float loc){
	glm::mat4 transAxle = glm::translate(glm::mat4(), glm::vec3(0.6, 0, loc));
	glMultMatrixf(&transAxle[0][0]);
	//Spin the axle while moving
	glm::mat4 spinAxle = glm::rotate(glm::mat4(), aRot, glm::vec3(1, 0, 0));
	glMultMatrixf(&spinAxle[0][0]);
	glm::mat4 rotAxle = glm::rotate(glm::mat4(), (float) M_PI/2.0f, glm::vec3(0, 0, 1));
	glMultMatrixf(&rotAxle[0][0]);
	CSCI441::drawSolidCylinder( 0.125, 0.125, 1.2, 20, 20);
	glm::mat4 rotDisk = glm::rotate(glm::mat4(), (float) M_PI/2.0f, glm::vec3(1, 0, 0));
	glMultMatrixf(&rotDisk[0][0]);
	glColor3f(0, 0, 0);
	CSCI441::drawSolidDisk( 0.001, 0.126, 20, 20);
	drawWheel();
	glm::mat4 tDisk  = glm::translate(glm::mat4(), glm::vec3(0, 0, -1.2));
	glMultMatrixf(&tDisk[0][0]);
	glColor3f(0, 0, 0);
	CSCI441::drawSolidDisk( 0.001, 0.126, 20, 20);
	drawWheel();
	glMultMatrixf(&(glm::inverse(tDisk))[0][0]);
	glMultMatrixf(&(glm::inverse(rotDisk))[0][0]);
	glMultMatrixf(&(glm::inverse(rotAxle))[0][0]);
	glMultMatrixf(&(glm::inverse(spinAxle))[0][0]);
	glMultMatrixf(&(glm::inverse(transAxle))[0][0]);
}

//Draw the vechicle the user moves around the city
void drawVehicle(){
	
	glm::mat4 rot = glm::rotate(glm::mat4(), vTheta, glm::vec3(0, 1, 0));
	glMultMatrixf(&rot[0][0]);
	drawBody();
	glColor3f(0.863, 0.078, 0.235);
	drawAxle(0.75);
	glColor3f(0.333, 0.420, 0.184);
	drawAxle(-0.75);
	glMultMatrixf(&(glm::inverse(rot))[0][0]);

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
void generateEnvironmentDL() {
    // TODO #1 Create a Display List & Call our Drawing Functions
	environmentDL = glGenLists(1);
	glNewList(environmentDL, GL_COMPILE);
		drawCity();
		drawGrid();
	glEndList();
}

//Draws the main ball of the mascot
void drawBall(){
	glColor3f(1.000, 0.714, 0.757);
	CSCI441::drawSolidSphere(0.5, 15, 15);
}

//Draws the spikes on the mascot
void drawSpike(){
	glm::mat4 rot = glm::rotate(glm::mat4(), (float) -M_PI/2.0f, glm::vec3(0, 0, 1));
	glMultMatrixf(&rot[0][0]);
	glColor3f(0.824, 0.412, 0.118);
	CSCI441::drawSolidCone(.1, 0.7, 15, 15);
	glMultMatrixf(&(glm::inverse(rot))[0][0]);
}

//Draws the mascot
void drawMascot(){
	drawBall();
	for(float i = 0.0f; i < 2.0f*M_PI; i += M_PI/4.0f){
		glm::mat4 rot = glm::rotate(glm::mat4(), i, glm::vec3(0, 1, 0));
		glMultMatrixf(&rot[0][0]);
		//Move the spikes in and out of the mascot
		glm::mat4 tran = glm::translate(glm::mat4(), glm::vec3(extend, 0, 0));
		glMultMatrixf(&tran[0][0]);
		drawSpike();
		glMultMatrixf(&(glm::inverse(tran))[0][0]);
		glMultMatrixf(&(glm::inverse(rot))[0][0]);
	}
}

//Draw the curve by connecting the points genereated in evaluateBezierCurve()
void drawCurve(){
		if(showControl){
			// Draw our control points
			for(int i = 0; i < (int) controlPoints.size(); i++){
				drawPoint(controlPoints.at(i));
			}
			// Connect our control points
			drawControl();
		}
		
		//Draw the Bezier Curve!
		for(int i = 0; i < (int)controlPoints.size()-1; i += 3){
					renderBezierCurve();
		}
}

//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene(void)  {
	glCallList(environmentDL);
	
	//Move vehical to the starting location (center of the curve in the x and z direction)
	glm::mat4 loc = glm::translate(glm::mat4(), glm::vec3(vLoc.x, vLoc.y, vLoc.z));
	glMultMatrixf(&loc[0][0]);
		drawVehicle();
	//center the curve's y location
	glm::mat4 curveY = glm::translate(glm::mat4(), glm::vec3(0, -center.y, 0));
	glMultMatrixf(&curveY[0][0]);
	//Rotate the cureve around the z axis
		glm::mat4 rotCurve = glm::rotate(glm::mat4(), curveRot, glm::vec3(0, 0, 1));
		glMultMatrixf(&rotCurve[0][0]);
		drawCurve();
		//Move the mascot along the curve
		glm::mat4 tran = glm::translate(glm::mat4(), mascotLoc);
		glMultMatrixf(&tran[0][0]);
			drawMascot();
		glMultMatrixf(&(inverse(tran)[0][0]));
		glMultMatrixf(&(inverse(rotCurve)[0][0]));
	glMultMatrixf(&(glm::inverse(curveY))[0][0]);
	glMultMatrixf(&(glm::inverse(loc))[0][0]);
	
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
GLFWwindow* setupGLFW() {
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback( error_callback );

	// initialize GLFW
	if( !glfwInit() ) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );	// request OpenGL v2.X
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	// request OpenGL v2.1
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		// do not allow our window to be able to be resized

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "Mascot", NULL, NULL );
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(window);		// make the created window the current window
	glfwSwapInterval(1);				     	// update our screen after at least 1 screen refresh

	glfwSetKeyCallback( window, keyboard_callback );							// set our keyboard callback function
	glfwSetCursorPosCallback( window, cursor_callback );					// set our cursor position callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function

	return window;						       // return the window that was created
}

//
//  void setupOpenGL()
//
//      Used to setup everything OpenGL related.  For now, the only setting
//	we need is what color to make the background of our window when we clear
//	the window.  In the future we will be adding many more settings to this
//	function.
//
void setupOpenGL() {
	// tell OpenGL to perform depth testing with the Z-Buffer to perform hidden
	//		surface removal.  We will discuss this more very soon.
  glEnable( GL_DEPTH_TEST );

	//******************************************************************
  // this is some code to enable a default light for the scene;
  // feel free to play around with this, but we won't talk about
  // lighting in OpenGL for another couple of weeks yet.
  float lightCol[4] = { 1, 1, 1, 1};
  float ambientCol[4] = { 0.0, 0.0, 0.0, 1.0 };
  float lPosition[4] = { 10, 10, 10, 1 };
  glLightfv( GL_LIGHT0, GL_POSITION,lPosition );
  glLightfv( GL_LIGHT0, GL_DIFFUSE,lightCol );
  glLightfv( GL_LIGHT0, GL_AMBIENT, ambientCol );
  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );

  // tell OpenGL not to use the material system; just use whatever we
	// pass with glColor*()
  glEnable( GL_COLOR_MATERIAL );
  glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
	//******************************************************************

  // tells OpenGL to blend colors across triangles. Once lighting is
  // enabled, this means that objects will appear smoother - if your object
  // is rounded or has a smooth surface, this is probably a good idea;
  // if your object has a blocky surface, you probably want to disable this.
  glShadeModel( GL_SMOOTH );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
}

//
//  void setupScene()
//
//      Used to setup everything scene related.  Give our camera an
//	initial starting point and generate the display list for our city
//
void setupScene() {
	// give the camera a scenic starting point.
	camZoom = 10.0f;
	cameraTheta = 2.0f*M_PI/3.0f;
	cameraPhi = M_PI / 1.5f;
	recomputeOrientation();
	
	//Give the vehicle a starting point
	vLoc.x = center.x;
	vLoc.y = 0.28;
	vLoc.z = center.z;
	vTheta = 0;
	recomputeVehicleDirection();

	//Set the initial mascot location
	mascotLoc = controlPoints[0];
	
	srand( time(NULL) );	// seed our random number generator
	generateEnvironmentDL();
}

//Move the mascot around the curve
void moveMascot(){
		if(currentPoint == (int) points.size()-1) currentPoint = 0;
		mascotLoc = points[currentPoint];
		currentPoint++;
		if(extend >= 0.7) extDir = -1.0f;
		if(extend <= 0.0) extDir = 1.0f;
		extend += 0.01*extDir;	
}

//Rotate the curve around the vehicle 
void rotateCurve(){
	if(curveRot >= 2.0*M_PI) curveRot = 0;
	curveRot += 0.01;
}

///*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main( int argc, char *argv[] ) {
	
	//Read control points from CSV file
	if(argc == 0){
			fprintf(stderr, "[ERROR]: No CSV file passed in\n");
		}else{
			if(!loadControlPoints(argv[1])){
				fprintf(stderr, "[ERROR]: Unable to open file or file formated incorrectly\n");
			}
		}
	// GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
	setupOpenGL();										// initialize all of the OpenGL specific information
	setupScene();											// initialize objects in our scene

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
    glDrawBuffer( GL_BACK );				// work with our back frame buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window
 		
		//Move the vehicle if it will stay within the boundry of the grid
		if(vMove != 0){
			glm::vec3 vLocTest = vLoc + (float) vMove*vDir*0.1f;
					if(vLocTest.x > -50 && vLocTest.x < 50 && vLocTest.z > -50 && vLocTest.z < 50){
						if(aRot > 2.0f*M_PI && vMove > 0) aRot = 0.0f;
						if(aRot < -2.0f*M_PI && vMove < 0) aRot = 0.0f;
						aRot -= vMove*0.07;
						vLoc = vLocTest;	 		
					}
		}
		
		moveMascot();
		rotateCurve();
		
		//Change the angle of the vehicle
		if(vRot != 0){
			vTheta += vRot*0.05f;
			recomputeVehicleDirection();
		}
		
		// update the projection matrix based on the window size
		// the GL_PROJECTION matrix governs properties of the view coordinates;
		// i.e. what gets seen - use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.001f, 1000.0f );
		glMatrixMode( GL_PROJECTION );	// change to the Projection matrix
		glLoadIdentity();				// set the matrix to be the identity
		glMultMatrixf( &projMtx[0][0] );// load our orthographic projection matrix into OpenGL's projection matrix state

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		GLint framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport( 0, 0, framebufferWidth, framebufferHeight );

		glMatrixMode( GL_MODELVIEW );	// make the ModelView matrix current to be modified by any transformations
		glLoadIdentity();							// set the matrix to be the identity

		// set up our look at matrix to position our camera
		glm::mat4 viewMtx = glm::lookAt( camPos + vLoc, vLoc, glm::vec3(  0,  1,  0 ));
		// multiply by the look at matrix - this is the same as our view martix
		glMultMatrixf( &viewMtx[0][0] );

		renderScene();					// draw everything to the window

		glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
