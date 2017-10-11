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
#include <sstream> // for string to int conversion

using namespace std;
float wheelAng = 0.0f;				   // angle of car's wheel
float carDir = 0.0f;				   // car direction in degrees
glm::vec3 carPos = glm::vec3(0, 0, 0); // car position

// Global for kduong
vector<glm::vec3> controlPointsk;
bool point = false;
bool connection = false;
bool curve = false;
float mascotAngle = 0.0f;
vector<glm::vec3> mascotPath;
int iter = 0;


// evaluateBezierCurve() ////////////////////////////////////////////////////////
//
// Computes a location along a Bezier Curve.
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 evaluateBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
	glm::vec3 point(0, 0, 0);

	// TODO #06: Compute a point along a Bezier curve
	float x = (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * (t * t * t) + (3.0f * p0.x - 6.0f * p1.x + 3.0f * p2.x) * (t * t) + (-3.0f * p0.x + 3.0f * p1.x) * t + p0.x;
	float y = (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * (t * t * t) + (3.0f * p0.y - 6.0f * p1.y + 3.0f * p2.y) * (t * t) + (-3.0f * p0.y + 3.0f * p1.y) * t + p0.y;
	float z = (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * (t * t * t) + (3.0f * p0.z - 6.0f * p1.z + 3.0f * p2.z) * (t * t) + (-3.0f * p0.z + 3.0f * p1.z) * t + p0.z;

	point = glm::vec3(x, y, z);

	return point;
}

// renderBezierCurve() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////
void renderBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int resolution)
{
	// TODO #05: Draw the Bezier Curve!
	glm::vec3 point(0, 0, 0);
	glm::vec3 nxt(0, 0, 0);

	// Might look weird, ran into trouble with <= because of float precision, resorted to just <
	point = evaluateBezierCurve(p0, p1, p2, p3, 0.0f);
	for (float t = 1.0f / (float)resolution; t < 1.0f + 1.0f / (float)resolution; t += 1.0f / (float)resolution)
	{
		nxt = evaluateBezierCurve(p0, p1, p2, p3, t);
		glBegin(GL_LINES);
		{
			glVertex3f(point.x, point.y, point.z);
			glVertex3f(nxt.x, nxt.y, nxt.z);
		};
		glEnd();
		point = nxt;
	}
}

void drawCurvek()
{
	// TODO #03: Draw our control points
	if (point)
	{
		for (unsigned int i = 0; i < controlPointsk.size(); i++)
		{
			glm::vec3 vec = controlPointsk.at(i);
			glm::mat4 transMtx = glm::translate(glm::mat4(), vec);
			glMultMatrixf(&transMtx[0][0]);
			{
				glColor3f(0, 1, 0);
				CSCI441::drawSolidSphere(.2, 10, 10);
			};
			glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
		}
	}

	// TODO #04: Connect our control points
	if (connection)
	{
		glDisable(GL_LIGHTING);
		for (unsigned int i = 0; i < controlPointsk.size() - 1; i++)
		{
			glm::vec3 v1 = controlPointsk.at(i);
			glm::vec3 v2 = controlPointsk.at(i + 1);
			glLineWidth(5.0f);
			glBegin(GL_LINES);
			{
				glColor3f(1, 1, 0);
				glVertex3f(v1.x, v1.y, v1.z);
				glVertex3f(v2.x, v2.y, v2.z);
			};
			glEnd();
			glLineWidth(1.0f);
		}
		glEnable(GL_LIGHTING);
	}

	// TODO #05: Draw the Bezier Curve!
	if (curve)
	{
		glDisable(GL_LIGHTING);
		glLineWidth(5.0f);
		glColor3f(0, 0, 1);
		for (unsigned int i = 0; i < controlPointsk.size() - 3; i += 3)
		{
			glm::vec3 v1 = controlPointsk.at(i);
			glm::vec3 v2 = controlPointsk.at(i + 1);
			glm::vec3 v3 = controlPointsk.at(i + 2);
			glm::vec3 v4 = controlPointsk.at(i + 3);
			renderBezierCurve(v1, v2, v3, v4, 10);
		}
		glLineWidth(1.0f);
		glEnable(GL_LIGHTING);
	}
}

void populatePath()
{
	int maxSegment = 100;
	for (unsigned int i = 0; i < controlPointsk.size() - 3; i += 3)
	{
		glm::vec3 v1 = controlPointsk.at(i);
		glm::vec3 v2 = controlPointsk.at(i + 1);
		glm::vec3 v3 = controlPointsk.at(i + 2);
		glm::vec3 v4 = controlPointsk.at(i + 3);
		// Might look weird, ran into trouble with <= because of float precision, resorted to just <
		for (int j = 0; j < maxSegment; j++)
		{
			float t = (float)j / (float)maxSegment;
			glm::vec3 nxt = evaluateBezierCurve(v1, v2, v3, v4, t);
			mascotPath.push_back(nxt);
		}
	}
}

void populatePath(vector<glm::vec3> &cPoints, vector<glm::vec3> &cPath)
{
	int maxSegment = 200;
	for (unsigned int i = 0; i < cPoints.size() - 3; i += 3)
	{
		glm::vec3 v1 = cPoints.at(i);
		glm::vec3 v2 = cPoints.at(i + 1);
		glm::vec3 v3 = cPoints.at(i + 2);
		glm::vec3 v4 = cPoints.at(i + 3);
		// Might look weird, ran into trouble with <= because of float precision, resorted to just <
		for (int j = 0; j < maxSegment; j++)
		{
			float t = (float)j / (float)maxSegment;
			glm::vec3 nxt = evaluateBezierCurve(v1, v2, v3, v4, t);
			cPath.push_back(nxt);
		}
	}
}

//*************************************************************************************
//
bool loadControlPointsKD(char *filename)
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

		controlPointsk.push_back(glm::vec3(x, y, z));
	}
	input.close();
	populatePath();

	return true;
}

bool loadControlPointsKD(char *filename, vector<glm::vec3> &cPoints, vector<glm::vec3> &cPath)
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

		cPoints.push_back(glm::vec3(x, y, z));
	}
	input.close();
	populatePath(cPoints, cPath);

	return true;
}

void drawChassis()
{
	glm::mat4 transMtx = glm::translate(glm::mat4(), glm::vec3(0, 1, 0));
	glMultMatrixf(&transMtx[0][0]);
	{
		glm::mat4 rotMtx = glm::rotate(glm::mat4(), 3.14f / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		glMultMatrixf(&rotMtx[0][0]);
		{
			//glColor3f(113.0/255.0,188.0/255.0,130.0/255.0);
			glColor3f(.2, 1, 1);
			CSCI441::drawSolidTeapot(1);
		};
		glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
}

void drawWheelk()
{
	glm::mat4 rotMtx = glm::rotate(glm::mat4(), 3.14f / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glMultMatrixf(&rotMtx[0][0]);
	{
		glColor3f(1, 0, 0);
		CSCI441::drawSolidTorus(.2, .3, 3, 3);
	};
	glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
}

void drawWheels()
{
	glm::mat4 transMtx = glm::translate(glm::mat4(), glm::vec3(1, .3, 1));
	glMultMatrixf(&transMtx[0][0]);
	{
		glm::mat4 rotMtx = glm::rotate(glm::mat4(), wheelAng * 3.14f / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		glMultMatrixf(&rotMtx[0][0]);
		{
			drawWheelk();
		};
		glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(1, .3, -1));
	glMultMatrixf(&transMtx[0][0]);
	{
		glm::mat4 rotMtx = glm::rotate(glm::mat4(), wheelAng * 3.14f / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		glMultMatrixf(&rotMtx[0][0]);
		{
			drawWheelk();
		};
		glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(-1, .3, 1));
	glMultMatrixf(&transMtx[0][0]);
	{
		glm::mat4 rotMtx = glm::rotate(glm::mat4(), wheelAng * 3.14f / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		glMultMatrixf(&rotMtx[0][0]);
		{
			drawWheelk();
		};
		glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(-1, .3, -1));
	glMultMatrixf(&transMtx[0][0]);
	{
		glm::mat4 rotMtx = glm::rotate(glm::mat4(), wheelAng * 3.14f / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		glMultMatrixf(&rotMtx[0][0]);
		{
			drawWheelk();
		};
		glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(-1, .3, 0));
	glMultMatrixf(&transMtx[0][0]);
	{
		glm::mat4 rotMtx = glm::rotate(glm::mat4(), wheelAng * 3.14f / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		glMultMatrixf(&rotMtx[0][0]);
		{
			drawWheelk();
		};
		glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(1, .3, 0));
	glMultMatrixf(&transMtx[0][0]);
	{
		glm::mat4 rotMtx = glm::rotate(glm::mat4(), wheelAng * 3.14f / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		glMultMatrixf(&rotMtx[0][0]);
		{
			drawWheelk();
		};
		glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
}

void drawSuspension()
{
	glm::mat4 scaleMtx = glm::scale(glm::mat4(), glm::vec3(1, .1, 1));
	glMultMatrixf(&scaleMtx[0][0]);
	{
		glm::mat4 transMtx = glm::translate(glm::mat4(), glm::vec3(0, 2, 0));
		glMultMatrixf(&transMtx[0][0]);
		{
			//glColor3f(216.0/255.0,101.0/255.0,101.0/255.0);
			glColor3f(1, 1, 1);
			CSCI441::drawSolidCube(2);
		};
		glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(scaleMtx))[0][0]);
}

void drawCar()
{
	drawChassis();

	drawWheels();

	drawSuspension();
}

void drawBodyk()
{
	glColor3f(252.0f / 255.0f, 206.0f / 255.0f, 106.0f / 255.0f);
	CSCI441::drawSolidSphere(1, 10, 10);
}

void drawEyes()
{
	glColor3f(.8, .23, .23);
	glm::mat4 transMtx = glm::translate(glm::mat4(), glm::vec3(.4, .5, 0));
	glMultMatrixf(&transMtx[0][0]);
	{
		CSCI441::drawSolidSphere(.2, 5, 5);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(-.4, .5, 0));
	glMultMatrixf(&transMtx[0][0]);
	{
		CSCI441::drawSolidSphere(.2, 5, 5);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
}

void drawEars()
{
	glColor3f(.4, .22, .09);
	glm::mat4 rotMtx = glm::rotate(glm::mat4(), 45.0f * 3.14f / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glMultMatrixf(&rotMtx[0][0]);
	{
		glm::mat4 transMtx = glm::translate(glm::mat4(), glm::vec3(-.6, 0, 0));
		glMultMatrixf(&transMtx[0][0]);
		{
			CSCI441::drawSolidCone(.5, 1, 10, 10);
		};
		glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);

	rotMtx = glm::rotate(glm::mat4(), -45.0f * 3.14f / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glMultMatrixf(&rotMtx[0][0]);
	{
		glm::mat4 transMtx = glm::translate(glm::mat4(), glm::vec3(.6, 0, 0));
		glMultMatrixf(&transMtx[0][0]);
		{
			CSCI441::drawSolidCone(.5, 1, 10, 10);
		};
		glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
}

void drawMouth()
{
	glColor3f(.9, .5, .5);
	CSCI441::drawSolidPartialDisk(.3, .5, 10, 10, 0, 360);
}

void drawTail()
{
	glColor3f(1, .22, .09);
	glm::mat4 scaleMtx = glm::scale(glm::mat4(), glm::vec3(1, 1, 10));
	glMultMatrixf(&scaleMtx[0][0]);
	{
		CSCI441::drawSolidCube(.2);
	};
	glMultMatrixf(&(glm::inverse(scaleMtx))[0][0]);
}

void drawMascotk()
{
	glm::mat4 transMtx = glm::translate(glm::mat4(), glm::vec3(0, 0, .8));
	glMultMatrixf(&transMtx[0][0]);
	{
		drawEyes();
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(0, 1, 0));
	glMultMatrixf(&transMtx[0][0]);
	{
		drawEars();
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(0, -.2, .95));
	glMultMatrixf(&transMtx[0][0]);
	{
		drawMouth();
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	transMtx = glm::translate(glm::mat4(), glm::vec3(0, 0, -.8));
	glMultMatrixf(&transMtx[0][0]);
	{
		drawTail();
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	drawBodyk();
}

void drawKD()
{

	glm::mat4 transMtx = glm::translate(glm::mat4(), carPos);
	glMultMatrixf(&transMtx[0][0]);
	{

	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);

	// Draw car functions
	transMtx = glm::translate(glm::mat4(), carPos);
	glMultMatrixf(&transMtx[0][0]);
	{
		glm::mat4 rotMtx = glm::rotate(glm::mat4(), carDir * 3.14f / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		glMultMatrixf(&rotMtx[0][0]);
		{
			drawCar();
			drawCurvek();

			glm::mat4 pathMtx = glm::translate(glm::mat4(), mascotPath.at(iter));
			glMultMatrixf(&pathMtx[0][0]);
			{
				glm::mat4 barrellMtx = glm::rotate(glm::mat4(), mascotAngle, glm::vec3(0.0f, 0.0f, 1.0f));
				glMultMatrixf(&barrellMtx[0][0]);
				{
					drawMascotk();
				};
				glMultMatrixf(&(glm::inverse(barrellMtx))[0][0]);
			};
			glMultMatrixf(&(glm::inverse(pathMtx))[0][0]);

			// Mascots transformations
		};
		glMultMatrixf(&(glm::inverse(rotMtx))[0][0]);
	};
	glMultMatrixf(&(glm::inverse(transMtx))[0][0]);
}